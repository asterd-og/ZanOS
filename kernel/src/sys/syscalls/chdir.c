#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_chdir(syscall_args a) {
  char* path = (char*)a.arg1;
  process* proc = this_proc();
  vfs_node* node = vfs_open(proc->current_dir, path);
  if (!node || node->type != VFS_DIRECTORY)
    return (u64)-1;
  proc->current_dir = node;
  return 0;
}