#include <sys/syscall.h>
#include <sys/smp.h>
#include <mm/shmem.h>

// Share memory.
u64 syscall_shmem(syscall_args a) {
  uptr addr = (uptr)a.arg1;
  u64 size = (u64)a.arg2;
  u64 flags = (u64)a.arg3;
  u64 max_conn = (u64)a.arg4;
  if (!addr) return (u64)NULL;
  if (!size) return (u64)NULL;

  return shmem_create(addr, size, max_conn, flags);
}

u64 syscall_shmem_connect(syscall_args a) {
  u64 id = (u64)a.arg1;
  return (u64)shmem_connect(id);
}

u64 syscall_shmem_get(syscall_args a) {
  u64 id = (u64)a.arg1;
  return (u64)shmem_get(id);
}