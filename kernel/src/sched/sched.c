#include <sched/sched.h>
#include <sys/smp.h>
#include <sys/cpu.h>
#include <lib/lock.h>
#include <lib/elf.h>
#include <dev/pit.h>
#include <dev/dev.h>
#include <dev/tty.h>
#include <dev/char/keyboard.h>
#include <fs/vfs.h>
#include <mm/malloc.h>

u64 sched_glob_id = 0;
atomic_lock sched_lock;

void sched_idle() {
  while (1) {
    __asm__ volatile ("hlt");
  }
}

void sched_init() {
  cpu_info* cpu = this_cpu();
  cpu->task_idx = -1;
  cpu->task_current = NULL;
  sched_new_task(sched_idle, cpu->lapic_id, true);
}

task_ctrl* sched_new_task(void* entry, u64 cpu, bool idle) {
  cpu_info* c = get_cpu(cpu);
  lock(&c->sched_lock);

  task_ctrl* task = (task_ctrl*)kmalloc(sizeof(task_ctrl));
  memset(task, 0, sizeof(task_ctrl));

  task->idle = idle;
  task->user = false;

  char* stack = (char*)kmalloc(2 * PAGE_SIZE);
  task->ctx.rsp = (u64)(stack + (2 * PAGE_SIZE));
  task->ctx.rip = (u64)entry;
  task->ctx.cs  = 0x28;
  task->ctx.ss  = 0x30;
  task->ctx.rflags = 0x202;

  task->pm = vmm_new_pm();
  task->heap_area = heap_create(task->pm);

  __asm__ volatile ("fxsave %0" : : "m"(task->fxsave));

  task->cpu_idx = c->task_count;

  task->id = (idle ? 0 : ++sched_glob_id);
  task->cpu = cpu;
  task->stack_base = (u64)stack;
  task->sleeping_time = 0;
  task->state = SCHED_RUNNING;

  task->current_dir = vfs_root;

  c->task_list[c->task_count++] = task;

  unlock(&c->sched_lock);

  return task;
}

task_ctrl* sched_new_elf(char* path, u64 cpu, int argc, char** argv) {
  cpu_info* c = get_cpu(cpu);
  lock(&c->sched_lock);

  task_ctrl* task = (task_ctrl*)kmalloc(sizeof(task_ctrl));
  memset(task, 0, sizeof(task_ctrl));

  task->pm = vmm_new_pm();
  task->heap_area = heap_create(task->pm);

  task->idle = false;

  vfs_node* node = vfs_open(vfs_root, path);
  u32 size = node->size;
  u8* img = (u8*)kmalloc(size);
  dprintf("sched_new_elf(): Loading elf '%s' with %u bytes.\n", node->name, size);
  vfs_read(node, img, size);
  u64 entry = elf_load(img, task->pm);
  if (entry == (u64)-1) {
    dprintf("sched_new_elf(): Failed to load elf.\n");
    kfree(task);
    unlock(&c->sched_lock);
    return NULL;
  }

  task->ctx.rip = (u64)entry;
  task->ctx.cs  = 0x43;
  task->ctx.ss  = 0x3B;
  task->ctx.rflags = 0x202;
  task->kernel_stack = (u64)(kmalloc(3 * PAGE_SIZE)) + (3 * PAGE_SIZE);

  __asm__ volatile ("fxsave %0" : : "m"(task->fxsave));

  task->ctx.rdi = argc + 1; // argc

  pagemap* bp = this_cpu()->pm;
  vmm_switch_pm(task->pm);

  char* stack = (char*)heap_alloc(task->heap_area, 3 * PAGE_SIZE);
  task->ctx.rsp = (u64)(stack + (3 * PAGE_SIZE));
  task->stack_base = (u64)stack;
  memset(stack, 0, 3 * PAGE_SIZE);

  char** argv_user = (char**)heap_alloc(task->heap_area, (argc + 1) * sizeof(char*));
  argv_user[0] = (char*)heap_alloc(task->heap_area, strlen(node->name));
  memcpy(argv_user[0], node->name, strlen(node->name));

  if (argc > 0) {
    for (int i = 0; i < argc; i++) {
      int arg_len = strlen(argv[i]) + 1;
      argv_user[i + 1] = (char*)heap_alloc(task->heap_area, arg_len);
      memcpy(argv_user[i + 1], argv[i], arg_len);
    }
  }

  vmm_switch_pm(bp);

  task->ctx.rsi = (u64)argv_user;

  task->cpu_idx = c->task_count;

  task->id = ++sched_glob_id;
  task->cpu = cpu;
  task->sleeping_time = 0;
  task->state = SCHED_RUNNING;

  task->current_dir = node->parent;
  task->fds[0] = fd_open(kb_node, FS_READ, 0); // stdin
  task->fds[1] = fd_open(tty_node, FS_READ | FS_WRITE, 1); // stdout
  task->fds[2] = fd_open(tty_node, FS_READ | FS_WRITE, 2); // stderr
  task->fd_idx = 3;

  c->task_list[c->task_count++] = task;

  dprintf("sched_new_elf(): Task %ld created.\n", task->id);

  unlock(&c->sched_lock);

  return task;
}

task_ctrl* sched_get_next_task(cpu_info* c) {
  c->task_idx++;
  if (c->task_idx == c->task_count)
    c->task_idx = 0;
  
  task_ctrl* task = c->task_list[c->task_idx];
  while (task->state != SCHED_RUNNING) {
    if (task->state == SCHED_SLEEPING) {
      if (task->sleeping_time <= pit_ticks) {
        task->state = SCHED_RUNNING; // change this to block and unblock task to jump to it
        return task;
      }
    }

    c->task_idx++;
    if (c->task_idx == c->task_count)
      c->task_idx = 0;

    task = c->task_list[c->task_idx];
  }
  return task;
}

void sched_schedule(registers* r) {
  lapic_stop_timer();

  cpu_info* c = this_cpu();

  if (c->task_current != NULL) {
    c->task_current->ctx = *r;
    c->task_current->gs = read_kernel_gs();
    c->task_current->pm = this_cpu()->pm;
    __asm__ volatile ("fxsave %0" : : "m"(c->task_current->fxsave));
  }

  c->task_current = sched_get_next_task(c);
  *r = c->task_current->ctx;
  vmm_switch_pm(c->task_current->pm);
  __asm__ volatile ("fxrstor %0" : : "m"(c->task_current->fxsave));

  write_kernel_gs((u64)c->task_current);

  lapic_eoi();
  lapic_oneshot(0x80, 5); // 5ms timeslice
}

void yield() {
  __asm__ volatile ("cli");
  lapic_stop_timer();
  lapic_ipi(lapic_get_id(), 0x80);
  __asm__ volatile ("sti");
}

void block() {
  cpu_info* c = this_cpu();
  lapic_stop_timer();
  lapic_ipi(c->lapic_id, 0x80);
  c->task_current->state = SCHED_BLOCKED;
  __asm__ volatile ("sti");
}

void unblock(task_ctrl* task) {
  cpu_info* c = this_cpu();
  lapic_stop_timer();
  lapic_ipi(c->lapic_id, 0x80);
  task->state = SCHED_RUNNING;
  c->task_idx = task->cpu_idx-1;
  __asm__ volatile ("sti");
}

void sleep(u64 ms) {
  lapic_stop_timer();
  lapic_ipi(this_cpu()->lapic_id, 0x80);
  cpu_info* c = this_cpu();
  c->task_current->sleeping_time = pit_ticks + ms;
  c->task_current->state = SCHED_SLEEPING;
  __asm__ volatile ("sti");
}

void sched_kill(task_ctrl* task) {
  if (task->state == SCHED_DEAD) return;
  block();
}

task_ctrl* sched_get_task(u64 tid) {
  cpu_info* c;
  for (u64 id = 0; id < smp_cpu_count; id++) {
    c = get_cpu(id);
    for (u64 t = 0; t < c->task_count; t++)
      if (c->task_list[t]->id == tid)
        return c->task_list[t];
  }
  return NULL;
}

void sched_exit(int status) {
  lapic_stop_timer();
  cpu_info* c = this_cpu();
  c->task_current->exit_status = status;
  c->task_current->state = SCHED_DEAD;
  lapic_ipi(this_cpu()->lapic_id, 0x80);
  __asm__ volatile ("sti");
}
