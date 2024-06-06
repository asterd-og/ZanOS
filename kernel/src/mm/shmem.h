#pragma once

#include <types.h>

#define SHMEM_MAX 4096

typedef struct {
  u64 id;
  uptr addr;
  usize size;
  u64 flags;
  u16 conn_count;
  u16 max_conn;
} shmem_chunk;

u64 shmem_create(uptr addr, usize size, u16 max_conn, u64 flags);
int shmem_connect(u64 id);
void* shmem_get(u64 id);