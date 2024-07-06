#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 get_rip();

u64 syscall_fork(syscall_args a) {
  (void)a;
  thread* t = this_thread();
  process* parent = t->parent;

  u64 rsp = 0;
  u64 rbp = a.r->rbp;
  u64 rip = a.r->rcx;

  __asm__ volatile ("mov %%gs:0, %0" : "=r"(rsp));

  // TODO: Choose CPU automatically
  process* proc = sched_new_proc(parent->name, parent->type, parent->cpu, true);

  proc->pm = vmm_clone(parent->pm);
  for (u64 i = 0; i < parent->fd_idx; i++)
    proc->fds[i] = fd_open(parent->fds[i].vnode, parent->fds[i].mode, parent->fds[i].fd_num);
  proc->fd_idx = parent->fd_idx;
  proc->current_dir = parent->current_dir;

  thread* child = proc_add_thread(proc, (void*)rip, true);
  if (!child)
    return (u64)-1;
  memcpy(&child->ctx, a.r, sizeof(registers));
  child->ctx.rsp = rsp;
  child->ctx.rbp = rbp;
  child->ctx.cs  = 0x43;
  child->ctx.ss  = 0x3b;
  child->ctx.rflags = a.r->r11;
  child->ctx.rax = 0;
  child->ctx.rip = rip;

  heap_clone(t->heap_area, child->heap_area);

  u64 kstack_base = 0;
  __asm__ volatile ("mov %%gs:8, %0" : "=r"(kstack_base));

  child->stack_base = rsp;

  child->stack_bottom = t->stack_bottom;

  memcpy(child->fxsave, t->fxsave, 512);
  child->state = SCHED_RUNNING;

  return proc->pid;
}

u64 syscall_getpid(syscall_args a) {
  (void)a;
  return this_proc()->pid;
}
