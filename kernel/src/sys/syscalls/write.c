#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_write(syscall_args a) {
  u64 fd = (u64)a.arg1;
  void* buffer = a.arg2;
  size_t size = (size_t)a.arg3;
  process* proc = this_proc();
  
  if (!proc->fds[fd].vnode)
    return (u64)-1;

  int inc = vfs_write(proc->fds[fd].vnode, buffer, size);
  return inc;
}