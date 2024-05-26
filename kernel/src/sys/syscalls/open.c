#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_open(syscall_args a) {
  char* filename = (char*)a.arg1;
  u64 flags = (u64)a.arg2;
  task_ctrl* task = this_cpu()->task_current;

  if (!filename) return (u64)-1;

  dprintf("Opening %s\n", filename);

  vfs_node* node = vfs_open(task->current_dir, filename);
  if (!node) return (u64)-1;

  u64 fd = task->fd_idx;
  task->fds[fd] = fd_open(node, flags, fd);
  task->fd_idx++;
  dprintf("Opened %s %d\n", node->name, node->size);

  return fd;
}