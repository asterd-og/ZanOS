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

void syscall_handle(registers* r);

u64 syscall_exit(syscall_args a);
u64 syscall_kill(syscall_args a);
u64 syscall_signal(syscall_args a);
u64 syscall_raise(syscall_args a);

u64 syscall_sleep(syscall_args a);

u64 syscall_malloc(syscall_args a);
u64 syscall_free(syscall_args a);
u64 syscall_realloc(syscall_args a);

u64 syscall_getsz(syscall_args a);

u64 syscall_open(syscall_args a);
u64 syscall_read(syscall_args a);
u64 syscall_write(syscall_args a);
u64 syscall_close(syscall_args a);

u64 syscall_getcwd(syscall_args a);
u64 syscall_chdir(syscall_args a);

u64 syscall_opendir(syscall_args a);
u64 syscall_readdir(syscall_args a);
u64 syscall_closedir(syscall_args a);

u64 syscall_sock_new(syscall_args a);
u64 syscall_sock_bind(syscall_args a);
u64 syscall_sock_connect(syscall_args a);
u64 syscall_sock_accept(syscall_args a);
u64 syscall_sock_get_ready(syscall_args a);