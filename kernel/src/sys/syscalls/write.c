#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_write(syscall_args a) {
  int fd = (int)a.arg1;
  void* buffer = a.arg2;
  size_t size = (size_t)a.arg3;
  task_ctrl* task = this_cpu()->task_current;
  
  if (!task->fds[fd].vnode == NULL)
    return -1;
  
  return vfs_write(task->fds[fd].vnode, buffer, size);
}