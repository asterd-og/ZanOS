#include <sched/signal.h>
#include <sys/smp.h>

signal_handler sig_signal(int sig_no, signal_handler handler) {
  thread* thread = this_thread();
  signal_handler old = thread->sigs[sig_no];
  thread->sigs[sig_no] = handler;
  return old;
}

int sig_raise(int signal) {
  thread* thread = this_thread();
  if (thread->sigs[signal] != NULL) thread->sigs[signal](signal);
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
  process* proc = this_proc();
  printf("\033[38;2;255;0;0mSignal %d (%s) caught on proc %lu.\033[0m\n", signal, signals_to_str[signal], proc->pid);
  this_thread()->state = SCHED_DEAD;
  lapic_ipi(this_cpu()->lapic_id, 0x80);
  __asm__ volatile ("sti");
}