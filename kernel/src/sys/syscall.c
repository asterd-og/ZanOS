#include <sys/syscall.h>
#include <sched/signal.h>
#include <lib/libc.h>
#include <kernel.h>

void* syscall_table[] = {
  syscall_exit,     // 0
  syscall_kill,     // 1
  syscall_signal,   // 2
  syscall_raise,    // 3

  syscall_sleep,    // 4

  syscall_malloc,   // 5
  syscall_free,     // 6
  syscall_realloc,  // 7

  syscall_fopen,    // 8
  syscall_fread,    // 9
  syscall_fwrite,   // 10
  syscall_fclose,   // 11
  
  syscall_read,     // 12
  syscall_write,    // 13
  syscall_close,    // 14
};

void syscall_handle(registers* r) {
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