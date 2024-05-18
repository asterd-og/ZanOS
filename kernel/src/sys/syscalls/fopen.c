#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_fopen(syscall_args a) {
  const char* pathname = (char*)a.arg1;
  const char* mode = (char*)a.arg2;
  task_ctrl* task = this_cpu()->task_current;
  if (task->fd_idx >= 256) return -1;
  u16 flag = 0;

  if (mode[0] == 'r')
    flag |= FS_READ;
  else if (mode[0] == 'w') {
    flag |= FS_WRITE;
    flag |= FS_CREATE;
  }

  if (mode[1] == '+') {
    if (flag & FS_READ)
      flag |= FS_WRITE;
    else if (flag & FS_WRITE)
      flag |= FS_READ;
  } // Only r, w, r+ and w+ for now

  vfs_node* node = vfs_open(task->current_dir, pathname);
  if (!node)
    return -1;
  
  u64 fd = task->fd_idx;
  task->fds[fd] = fd_open(node, flag, fd);
  task->fd_idx++;

  return &task->fds[fd];
}