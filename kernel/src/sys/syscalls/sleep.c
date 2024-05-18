#include <sys/syscall.h>
#include <sched/sched.h>

u64 syscall_sleep(syscall_args a) {
  u64 ms = (u64)a.arg1;
  sleep(ms);
  return 0;
}