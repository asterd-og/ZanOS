#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_open(syscall_args a) {
  char* filename = (char*)a.arg1;
  u64 flags = (u64)a.arg2;
  process* proc = this_proc();

  if (!filename) return (u64)-1;

  vfs_node* node = vfs_open(proc->current_dir, filename);
  if (!node) return (u64)-1;

  u64 fd = proc->fd_idx;
  proc->fds[fd] = fd_open(node, flags, fd);
  proc->fd_idx++;

  return fd;
}