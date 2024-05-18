#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_fread(syscall_args a) {
  void* ptr = a.arg1;
  size_t size = (size_t)a.arg2;
  size_t nmemb = (size_t)a.arg3;
  file_descriptor* file = (file_descriptor*)a.arg4;
  if (!file) return -1;
  if (!(file->mode & FS_READ)) return -1;
  i32 ret = vfs_read(file->vnode, ptr, size * nmemb);
  return ret;
}