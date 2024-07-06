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

u64 sched_pid = 0;
atomic_lock sched_lock;
list* sched_sleep_list = NULL;

void sched_idle() {
  while (1) {
    __asm__ volatile ("hlt");
  }
}

void sched_init() {
  if (!sched_sleep_list)
    sched_sleep_list = list_create();
  cpu_info* cpu = this_cpu();
  cpu->idle_proc = sched_new_proc("Idle", SCHED_IDLE, cpu->lapic_id, false);
  proc_add_thread(cpu->idle_proc, sched_idle, false);
  cpu->proc_idx = -1;
  cpu->thread = NULL;
  cpu->proc = NULL;
}

process* sched_new_proc(char* name, u8 type, u64 cpu, bool child) {
  cpu_info* c = get_cpu(cpu);
  if (!c) return NULL;

  process* proc = (process*)kmalloc(sizeof(process));
  if (!proc) {
    return NULL;
  }
  memset(proc, 0, sizeof(proc));

  proc->cpu = cpu;

  proc->pm = (child ? NULL : vmm_new_pm());

  proc->type = type;

  proc->current_dir = vfs_root;
  proc->fds[0] = fd_open(kb_node, FS_READ, 0);
  proc->fds[1] = fd_open(tty_node, FS_WRITE, 1);
  proc->fds[2] = fd_open(tty_node, FS_WRITE, 2);

  proc->threads = list_create();

  proc->name = (char*)kmalloc(strlen(name));
  memcpy(proc->name, name, strlen(name));

  proc->tidx = -1;
  proc->scheduled = false;

  proc->idx = c->proc_list->count;
  proc->pid = sched_pid++; // TODO: hash this

  lock(&c->sched_lock);
  list_add(c->proc_list, proc);
  unlock(&c->sched_lock);

  dprintf("New proc %lu created.\n", proc->pid);
  return proc;
}

thread* proc_add_thread(process* proc, void* entry, bool fork) {
  thread* t = (thread*)kmalloc(sizeof(thread));
  if (!t) return NULL;
  memset(t, 0, sizeof(thread));

  t->pm = proc->pm;
  t->heap_area = heap_create(proc->pm);

  char* kstack = (char*)kmalloc(SCHED_STACK_SIZE);
  char* stack = NULL;

  if (proc->type == SCHED_USER && !fork) {
    pagemap* pm = this_cpu()->pm;
    vmm_switch_pm(t->pm);
    stack = (char*)heap_alloc(t->heap_area, SCHED_STACK_SIZE);
    memset(stack, 0, SCHED_STACK_SIZE);
    vmm_switch_pm(pm);
  } else if (!fork) {
    stack = (char*)kmalloc(SCHED_STACK_SIZE);
    memset(stack, 0, SCHED_STACK_SIZE);
  }

  t->stack_base = (u64)(stack + SCHED_STACK_SIZE);
  t->kernel_stack = (u64)(kstack + SCHED_STACK_SIZE);

  t->stack_bottom = (u64)stack;
  t->kstack_bottom = (u64)kstack;

  __asm__ volatile ("fxsave %0" : : "m"(t->fxsave));

  t->gs = 0;

  t->ctx.rip = (u64)entry;
  t->ctx.rsp = (u64)t->stack_base;
  t->ctx.cs  = (proc->type == SCHED_USER ? 0x43 : 0x28);
  t->ctx.ss  = (proc->type == SCHED_USER ? 0x3B : 0x30);
  t->ctx.rflags = 0x202;

  t->sleeping_time = 0;

  t->state = (fork ? SCHED_BLOCKED : SCHED_RUNNING);
  t->idx = proc->threads->count;
  t->parent = proc;

  lock(&proc->lock);
  list_add(proc->threads, t);
  unlock(&proc->lock);

  return t;
}

void proc_destroy_thread(thread* t) {
  kfree((void*)t->kstack_bottom);
  *((u64*)t->stack_bottom) = 0xdeadbeef; // Ensure copy-on-write
  free((void*)t->stack_bottom);
  // TODO: Destroy heap
  kfree(t);
}

void sched_destroy_proc(process* proc) {
  for (list_item* item = proc->threads->head->next; item != proc->threads->head; item = item->next) {
    proc_destroy_thread((thread*)item->val);
    list_remove(proc->threads, item);
  }
  list* proc_list = get_cpu(proc->cpu)->proc_list;
  list_item* item = list_find(proc_list, proc);
  if (!item) return;
  list_remove(proc_list, list_find(proc_list, proc));
  kfree(proc->name);
  vmm_switch_pm(vmm_kernel_pm);
  vmm_destroy_pm(proc->pm);
  kfree(proc);
}

thread* proc_add_elf_thread(process* proc, char* path) {
  thread* t = proc_add_thread(proc, NULL, false);
  vfs_node* node = vfs_open(proc->current_dir, path);
  u8* buffer = (u8*)kmalloc(node->size);
  vfs_read(node, buffer, node->size);
  t->ctx.rip = elf_load(buffer, t->pm);
  return t;
}

process* sched_get_next_proc(cpu_info* c) {
  lock(&c->sched_lock);
  process* proc;
  while (1) {
    proc = (process*)list_iterate(c->proc_list, true);
    if (proc->threads->count > 0)
      break;
  }
  unlock(&c->sched_lock);
  return proc;
}

thread* sched_get_next_thread(process* proc) {
  lock(&proc->lock);
  thread* t;
  while (1) {
    t = list_iterate(proc->threads, false);
    if (t == NULL) {
      get_cpu(proc->cpu)->scheduled_threads = true;
      break;
    }
    if (t->state == SCHED_RUNNING) {
      break;
    }
  }
  unlock(&proc->lock);
  return t;
}

void sched_schedule(registers* r) {
  lapic_stop_timer();

  cpu_info* c = this_cpu();

  if (c->thread) {
    thread* t = c->thread;
    t->ctx = *r;
    t->pm = c->pm;
    t->gs = read_kernel_gs();
    __asm__ volatile ("fxsave %0" : : "m"(t->fxsave));
  } else {
    c->proc = sched_get_next_proc(c);
  }

  // Find a suitable thread
  thread* next_thread = sched_get_next_thread(c->proc);
  while (c->scheduled_threads) {
    c->proc = sched_get_next_proc(c);
    next_thread = sched_get_next_thread(c->proc);
    c->scheduled_threads = false;
  }

  c->thread = next_thread;

  *r = c->thread->ctx;
  vmm_switch_pm(c->thread->pm);
  write_kernel_gs((u64)c->thread);

  __asm__ volatile ("fxrstor %0" : : "m"(c->thread->fxsave));

  lapic_eoi();
  lapic_oneshot(0x80, 5);
}

void yield() {
  __asm__ volatile("cli");
  lapic_stop_timer();
  lapic_eoi();
  lapic_ipi(this_cpu()->lapic_id, 0x80);
  __asm__ volatile("sti");
}

void sleep(u64 ms) {
  lapic_stop_timer();
  this_thread()->state = SCHED_SLEEPING;
  this_thread()->sleeping_time = ms;
  list_add(sched_sleep_list, this_thread());
  yield();
}

void unblock(thread* t) {
  cpu_info* c = get_cpu(t->parent->cpu);
  c->proc_idx = t->parent->idx - 1;
  t->parent->tidx = t->idx - 1;
  t->state = SCHED_RUNNING;
  c->proc->tidx = c->proc->threads->count - 1;
  lapic_ipi(c->lapic_id, 0x80);
}

void sched_exit(int status) {
  (void)status;
  this_thread()->state = SCHED_DEAD;
  sched_destroy_proc(this_proc());
  this_cpu()->proc = NULL;
  this_cpu()->thread = NULL;
  yield();
}

process* this_proc() {
  return this_cpu()->proc;
}

thread* this_thread() {
  return this_cpu()->thread;
}
