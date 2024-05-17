#include <fs/fs.h>
#include <stdarg.h>
#include <lib/printf.h>
#include <sched/sched.h>
#include <sys/smp.h>

file_descriptor fd_open(vfs_node* vnode) {
  file_descriptor fd;
  fd.vnode = vnode;
  fd.flags = 0;
  fd.offset = 0;
  return fd;
}

i32 fprintf(int fd, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  char buf[1024];
  int ret = vsprintf(buf, fmt, va);
  va_end(va);
  vfs_write(this_cpu()->task_current->fds[fd].vnode, buf, ret);
  return ret;
}