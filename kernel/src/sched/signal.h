#pragma once

#include <types.h>
#include <sched/sched.h>

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGABRT 5
#define SIGIOT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11

signal_handler sig_signal(int sig_no, signal_handler handler);
int sig_raise(int sig_no);