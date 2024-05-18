#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_fclose(syscall_args a) {
  task_ctrl* task = this_cpu()->task_current;
  file_descriptor* file = (file_descriptor*)a.arg1;
  if (!file) return -1;
  memset(task->fds + file->fd_num, 0, sizeof(file_descriptor));
  return 0;
}