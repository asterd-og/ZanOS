#include <sys/syscall.h>
#include <sched/signal.h>

u64 syscall_signal(syscall_args a) {
  signal_handler ret = sig_signal((int)a.arg1, (signal_handler)a.arg2);
  return (u64)&ret;
}

u64 syscall_raise(syscall_args a) {
  return (u64)sig_raise((int)a.arg1);
}