#include <sys/syscall.h>
#include <sched/sched.h>

u64 syscall_exit(syscall_args a) {
  int status = (int)a.arg1;
  sched_exit(status);
  return 0;
}