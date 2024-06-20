#include <sys/syscall.h>
#include <sched/sched.h>
#include <sched/signal.h>

u64 syscall_kill(syscall_args a) {
  u64 signal = (u64)a.arg2;
  sig_raise((signal ? signal : SIGKILL));
  //sched_kill(task);
  return 0;
}