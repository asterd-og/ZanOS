#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_read(syscall_args a) {
  u64 fd = (u64)a.arg1;
  void* buffer = a.arg2;
  size_t size = (size_t)a.arg3;
  task_ctrl* task = this_cpu()->task_current;
  
  if (!task->fds[fd].vnode)
    return -1;
  
  return vfs_read(task->fds[fd].vnode, buffer, size);
}