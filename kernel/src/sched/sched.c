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

u64 sched_glob_id = 0;
atomic_lock sched_lock;

void sched_idle() {
  while (1) {
  }
}

void sched_init() {
  cpu_info* cpu = this_cpu();
  cpu->task_idx = -1;
  sched_new_task(sched_idle, cpu->lapic_id, true);
}

task_ctrl* sched_new_task(void* entry, u64 cpu, bool idle) {
  lock(&sched_lock);

  cpu_info* c = get_cpu(cpu);

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
  task->heap_area = heap_create();

  task->id = (idle ? 0 : sched_glob_id++);
  task->cpu = cpu;
  task->stack_base = (u64)stack;
  task->sleeping_time = 0;
  task->state = SCHED_RUNNING;

  task->current_dir = vfs_root;

  c->task_list[c->task_count++] = task;

  unlock(&sched_lock);

  return task;
}

char** sched_create_argv(int argc, ...) {
  va_list va;
  va_start(va, argc);
  char** argv = (char**)kmalloc((argc + 1) * sizeof(char*));
  for (int i = 0; i < argc; i++) {
    char* arg = (char*)va_arg(va, char*);
    int arg_len = strlen(arg) + 1;
    argv[i + 1] = (char*)kmalloc(arg_len);
    memcpy(argv[i + 1], arg, arg_len);
  }
  va_end(va);
  return argv;
}

task_ctrl* sched_new_elf(char* path, u64 cpu, int argc, char** argv) {
  lock(&sched_lock);

  cpu_info* c = get_cpu(cpu);

  task_ctrl* task = (task_ctrl*)kmalloc(sizeof(task_ctrl));
  memset(task, 0, sizeof(task_ctrl));

  task->pm = vmm_new_pm();
  task->heap_area = heap_create();

  task->idle = false;
  task->idle = true;

  vfs_node* node = vfs_open(vfs_root, path);
  u32 size = node->size;
  u8* img = (u8*)kmalloc(size);
  dprintf("sched_new_elf(): Loading elf with %u bytes.\n", size);
  vfs_read(node, img, size);
  dprintf("sched_new_elf(): Elf read.\n");
  u64 entry = elf_load(img, task->pm);
  dprintf("sched_new_elf(): Elf loaded.\n");
  if (entry == (u64)-1) {
    dprintf("sched_new_elf(): Failed to load elf.\n");
    kfree(task);
    return NULL;
  }

  char* stack = (char*)pmm_alloc(3);
  task->ctx.rsp = (u64)(stack + (3 * PAGE_SIZE));
  task->ctx.rip = (u64)entry;
  task->ctx.cs  = 0x43;
  task->ctx.ss  = 0x3B;
  task->ctx.rflags = 0x202;

  vmm_map_user_range(task->pm, (uptr)stack, (uptr)stack, 3, PTE_PRESENT | PTE_WRITABLE | PTE_USER);

  task->ctx.rdi = argc + 1; // argc
  argv[0] = node->name;
  task->ctx.rsi = (u64)argv;

  vmm_map_user_range(task->pm, (uptr)argv, (uptr)PHYSICAL(argv), DIV_ROUND_UP(sizeof(argv), PAGE_SIZE), PTE_PRESENT | PTE_WRITABLE | PTE_USER);

  task->id = ++sched_glob_id;
  task->cpu = cpu;
  task->stack_base = (u64)stack;
  task->sleeping_time = 0;
  task->state = SCHED_RUNNING;

  task->current_dir = node->parent;
  task->fds[0] = fd_open(kb_node, FS_READ, 0); // stdin
  task->fds[1] = fd_open(tty_node, FS_READ | FS_WRITE, 1); // stdout
  task->fds[2] = fd_open(tty_node, FS_READ | FS_WRITE, 2); // stderr
  task->fd_idx = 3;

  c->task_list[c->task_count++] = task;

  dprintf("sched_new_elf(): Task %ld created.\n", task->id);

  unlock(&sched_lock);

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
        task->state = SCHED_RUNNING;
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

  if (c->task_current != NULL)
    c->task_current->ctx = *r;
  
  c->task_current = sched_get_next_task(c);
  vmm_switch_pm(c->task_current->pm);
  *r = c->task_current->ctx;

  lapic_eoi();
  lapic_oneshot(0x32, 5); // 5ms timeslice
}

void sleep(u64 ms) {
  cpu_info* c = this_cpu();
  c->task_current->sleeping_time = pit_ticks + ms;
  c->task_current->state = SCHED_SLEEPING;
  __asm__ volatile ("int $0x32"); // schedule and jump to the next task
}

void sched_block(task_ctrl* task, u8 reason) {
  cpu_info* c = get_cpu(task->cpu);
  task->state = reason;
  if (c->task_current == task)
    lapic_ipi(task->cpu, 0x32);
}

void sched_unblock(task_ctrl* task) {
  task->state = SCHED_RUNNING;
}

void sched_kill(task_ctrl* task) {
  if (task->state == SCHED_DEAD) return;
  task->state = SCHED_DEAD;
  if (this_cpu()->task_current == task)
    __asm__ volatile ("int $0x32");
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
  this_cpu()->task_current->exit_status = status;
  sched_kill(this_cpu()->task_current);
}
