#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_close(syscall_args a) {
  u64 fd = (u64)a.arg1;
  task_ctrl* task = this_cpu()->task_current;
  
  if (!task->fds[fd].vnode)
    return (u64)-1;
  
  vfs_destroy(task->fds[fd].vnode);
  task->fds[fd] = (file_descriptor){0, 0, 0, 0};

  return 0;
}