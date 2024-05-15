#include <sys/syscall.h>
#include <sched/signal.h>
#include <lib/libc.h>

void* syscall_table[] = {
  syscall_kill,
  syscall_signal,
  syscall_raise
};

void syscall_handler(registers* r) {
  if (syscall_table[r->rax] != NULL) {
    syscall_args args;
    args.arg1 = (void*)r->rdi;
    args.arg2 = (void*)r->rsi;
    args.arg3 = (void*)r->rdx;
    args.arg4 = (void*)r->r10;
    args.arg5 = (void*)r->r8;
    args.arg6 = (void*)r->r9;
    u64(*func)(syscall_args) = syscall_table[r->rax];
    r->rax = func(args);
  }
}