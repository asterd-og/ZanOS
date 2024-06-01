#include <sys/syscall.h>
#include <sched/signal.h>
#include <lib/libc.h>
#include <dev/char/serial.h>
#include <kernel.h>

u64 syscall_unhandled(syscall_args a) {
  (void)a;
  printf("PANIC! Unhandled syscall!\n");
  return 1;
}

void* syscall_table[] = {
  syscall_exit,         // 0
  syscall_kill,         // 1
  syscall_signal,       // 2
  syscall_raise,        // 3

  syscall_sleep,        // 4

  syscall_malloc,       // 5
  syscall_free,         // 6
  syscall_realloc,      // 7

  syscall_getsz,        // 8

  syscall_unhandled,    // 9
  syscall_unhandled,    // 10

  syscall_open,         // 11
  syscall_read,         // 12
  syscall_write,        // 13
  syscall_close,        // 14

  syscall_getcwd,       // 15
  syscall_chdir,        // 16

  syscall_opendir,      // 17
  syscall_readdir,      // 18
  syscall_closedir,     // 19

  syscall_sock_new,     // 20
  syscall_sock_connect, // 21
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