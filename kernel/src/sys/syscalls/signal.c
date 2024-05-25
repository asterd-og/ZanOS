#include <sys/syscall.h>
#include <sched/signal.h>

u64 syscall_signal(syscall_args a) {
  sig_signal((u64)a.arg1, (signal_handler)a.arg2);
  return 0;
}

u64 syscall_raise(syscall_args a) {
  return (u64)sig_raise((u64)a.arg1);
}