#include <sched/signal.h>
#include <sys/smp.h>

signal_handler sig_signal(int sig_no, signal_handler handler) {
  cpu_info* c = this_cpu();
  task_ctrl* task = c->task_list[c->task_idx];
  signal_handler old = task->sigs[sig_no];
  task->sigs[sig_no] = handler;
  return old;
}

int sig_raise(int signal) {
  cpu_info* c = this_cpu();
  task_ctrl* task = c->task_list[c->task_idx];
  if (task->sigs[signal] != NULL) task->sigs[signal](signal);
  else sig_def_handler(signal);
  return 0;
}

char* signals_to_str[] = {
  "?",
  "SIGHUP",
  "SIGINT",
  "SIGQUIT",
  "SIGILL",
  "SIGABRT",
  "SIGIOT",
  "SIGBUS",
  "SIGFPE",
  "SIGKILL",
  "SIGUSR1",
  "SIGSEGV",
  "SIGUSR2",
  "SIGPIPE",
  "SIGALRM",
  "SIGTERM"
};

void sig_def_handler(int signal) {
  cpu_info* c = this_cpu();
  printf("\033[38;2;255;0;0mSignal %d (%s) caught.\033[0m\n", signal, signals_to_str[signal]);
  sched_kill(c->task_list[c->task_idx]);
}