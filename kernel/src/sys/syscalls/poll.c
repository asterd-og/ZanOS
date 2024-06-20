#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <fs/vfs.h>

u64 syscall_poll(syscall_args a) {
  u64 fd = (u64)a.arg1;
  process* proc = this_proc();
  if (!proc->fds[fd].vnode)
    return 0;
  return vfs_poll(proc->fds[fd].vnode);
}