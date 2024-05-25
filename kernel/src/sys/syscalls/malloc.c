#include <sys/syscall.h>
#include <mm/heap.h>
#include <mm/malloc.h>

u64 syscall_malloc(syscall_args a) {
  u64 ret = (u64)malloc((u64)a.arg1);
  return ret;
}

u64 syscall_free(syscall_args a) {
  free(a.arg1);
  return 0;
}

u64 syscall_realloc(syscall_args a) {
  return (u64)realloc(a.arg1, (u64)a.arg2);
}