#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <fs/vfs.h>

u64 syscall_readdir(syscall_args a) {
  DIR* dir = (DIR*)a.arg1;
  if (!dir)
    return (u64)-1;
  vfs_dirent* dirent = vfs_readdir(dir->node, dir->current_index++);
  
  return (u64)dirent;
}