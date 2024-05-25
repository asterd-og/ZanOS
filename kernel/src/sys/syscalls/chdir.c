#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_chdir(syscall_args a) {
  char* path = (char*)a.arg1;
  task_ctrl* task = this_cpu()->task_current;
  vfs_node* node = vfs_open(task->current_dir, path);
  if (!node || node->type != VFS_DIRECTORY)
    return (u64)-1;
  task->current_dir = node;
  return 0;
}