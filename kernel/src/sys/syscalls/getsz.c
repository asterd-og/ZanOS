#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

typedef struct {
  u64 offset;
  u16 mode;
  u16 fd_num;
} syscall_file;

u64 syscall_getsz(syscall_args a) {
  syscall_file* file = (syscall_file*)a.arg1;
  if (!file) return (u64)-1;

  process* proc = this_proc();
  
  file_descriptor fd = proc->fds[file->fd_num];
  if (!fd.vnode) return (u64)-1;

  return fd.vnode->size;
}