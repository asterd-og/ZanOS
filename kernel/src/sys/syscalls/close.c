#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_close(syscall_args a) {
  int fd = (int)a.arg1;
  task_ctrl* task = this_cpu()->task_current;
  
  if (!task->fds[fd].vnode == NULL)
    return -1;
  
  vfs_destroy(task->fds[fd].vnode);
  memset(task->fds + fd, 0, sizeof(file_descriptor));

  return 0;
}