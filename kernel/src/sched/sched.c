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

void sched_idle() {
  while (1) {
    __asm__ volatile ("hlt");
  }
}

void sched_init() {
  cpu_info* cpu = this_cpu();
  cpu->idle_proc = sched_new_proc("Idle", sched_idle, SCHED_IDLE, cpu->lapic_id);
  cpu->proc_idx = -1;
  cpu->thread = NULL;
  cpu->proc = NULL;
}

process* sched_new_proc(char* name, void* entry, u8 type, u64 cpu) {
  cpu_info* c = get_cpu(cpu);
  if (!c) return NULL;

  process* proc = (process*)kmalloc(sizeof(process));
  if (!proc) return NULL;
  memset(proc, 0, sizeof(proc));

  proc->pid = sched_pid++; // TODO: hash this
  proc->cpu = cpu;

  proc->pm = vmm_new_pm();

  proc->type = type;

  proc->current_dir = vfs_root;

  proc->children = list_create();
  proc->threads = list_create();

  proc->name = (char*)kmalloc(strlen(name));
  memcpy(proc->name, name, strlen(name));

  proc->tidx = 0;
  proc->scheduled = false;

  proc_add_thread(proc, entry);
  list_add(c->proc_list, proc);

  dprintf("New proc %lu created.\n", proc->pid);
  return proc;
}

thread* proc_add_thread(process* proc, void* entry) {
  thread* t = (thread*)kmalloc(sizeof(thread));
  memset(t, 0, sizeof(thread));

  t->pm = proc->pm;
  t->heap_area = heap_create(t->pm);

  char* kstack = (char*)kmalloc(4 * PAGE_SIZE);
  char* stack = (char*)kmalloc(4 * PAGE_SIZE);

  t->stack_base = (u64)(stack + 4 * PAGE_SIZE);
  t->kernel_stack = (u64)(kstack + 4 * PAGE_SIZE);

  t->stack_bottom = (u64)stack;
  t->kstack_bottom = (u64)kstack;

  t->gs = 0;

  t->ctx.rip = (u64)entry;
  t->ctx.rsp = (u64)t->stack_base;
  t->ctx.cs  = 0x28;
  t->ctx.ss  = 0x30;
  t->ctx.rflags = 0x202;

  t->sleeping_time = 0;

  t->state = SCHED_RUNNING;

  list_add(proc->threads, t);

  return t;
}

process* sched_get_next_proc(cpu_info* c) {
  c->proc_idx++;
  if (c->proc_idx == c->proc_list->count) {
    c->proc_idx = -1;
    return c->idle_proc;
  }
  return (process*)list_get(c->proc_list, c->proc_idx);
}

thread* sched_get_next_thread(process* proc) {
  proc->tidx++;
  if (proc->tidx == proc->threads->count) {
    proc->tidx = -1;
    proc->scheduled = true;
    return NULL;
  }
  return (thread*)list_get(proc->threads, proc->tidx);
}

void sched_schedule(registers* r) {
  lapic_stop_timer();

  cpu_info* c = this_cpu();

  if (c->thread) {
    thread* t = c->thread;
    t->ctx = *r;
    t->pm = c->pm;
  } else
    c->proc = sched_get_next_proc(c);

  thread* next_thread = sched_get_next_thread(c->proc);

  while (next_thread == NULL) {
    // Probably has been scheduled already, or it just doesnt contain any threads
    // either way, just jump to the next proc
    c->proc = sched_get_next_proc(c);
    next_thread = sched_get_next_thread(c->proc);
  }

  c->thread = next_thread;

  *r = c->thread->ctx;
  vmm_switch_pm(c->thread->pm);

  lapic_eoi();
  lapic_oneshot(0x80, 5);
}

void yield() {
}

void sleep(u64 ms) {
  (void)ms;
}

process* this_proc() {
  return this_cpu()->proc;
}

thread* this_thread() {
  return this_cpu()->thread;
}