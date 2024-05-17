#pragma once

#include <types.h>
#include <sys/idt.h>

typedef struct {
  void* arg1;
  void* arg2;
  void* arg3;
  void* arg4;
  void* arg5;
  void* arg6;
} syscall_args;

void syscall_handler(registers* r);

u64 syscall_kill(syscall_args a);
u64 syscall_signal(syscall_args a);
u64 syscall_raise(syscall_args a);