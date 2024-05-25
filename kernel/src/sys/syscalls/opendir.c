#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <fs/vfs.h>
#include <mm/malloc.h>

u64 syscall_opendir(syscall_args a) {
  char* path = (char*)a.arg1;
  if (!path)
    return (u64)-1;

  DIR* dir = (DIR*)malloc(sizeof(DIR));
  dir->node = vfs_open(this_cpu()->task_current->current_dir, path);
  if (!dir->node) {
    free(dir);
    return (u64)NULL;
  }
  dir->current_index = 0;
  
  return (u64)dir;
}