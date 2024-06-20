#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_close(syscall_args a) {
  u64 fd = (u64)a.arg1;
  process* proc = this_proc();
  
  if (!proc->fds[fd].vnode)
    return (u64)-1;
  
  vfs_destroy(proc->fds[fd].vnode);
  proc->fds[fd] = (file_descriptor){0, 0, 0, 0};

  return 0;
}