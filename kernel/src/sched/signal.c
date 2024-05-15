#include <sched/signal.h>
#include <sys/smp.h>

signal_handler sig_signal(int sig_no, signal_handler handler) {
  cpu_info* c = this_cpu();
  task_ctrl* task = c->task_list[c->task_idx];
  signal_handler old = task->sigs[sig_no];
  task->sigs[sig_no] = handler;
  return old;
}

int sig_raise(int sig_no) {
  cpu_info* c = this_cpu();
  task_ctrl* task = c->task_list[c->task_idx];
  sched_kill(task, sig_no);
  return 0;
}