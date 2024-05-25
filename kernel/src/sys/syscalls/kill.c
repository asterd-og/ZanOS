#include <sys/syscall.h>
#include <sched/sched.h>
#include <sched/signal.h>

u64 syscall_kill(syscall_args a) {
  task_ctrl* task = sched_get_task((u64)a.arg1);
  if (task == NULL)
    return 1;
  u64 signal = (u64)a.arg2;
  sig_raise((signal ? signal : SIGKILL));
  sched_kill(task);
  return 0;
}