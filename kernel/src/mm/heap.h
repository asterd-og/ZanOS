#pragma once

#include <types.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#define HEAP_MAGIC 0xdeadbeef

typedef struct heap_pool {
  struct heap_pool* next;
  struct heap_pool* prev;
  u8* pool;
  uptr last_ptr;
  u64 free_size;
  u16 id;
  u64 size;
  bool used; // Only true if it has reached maximum capacity
} heap_pool;

typedef struct {
  heap_pool* pool_head;
  u64 size;
  u64 id;
  u16 pool_count;
} heap_ctrl;

heap_ctrl* heap_create(u64 size);

void* heap_alloc(heap_ctrl* h, u64 size);
void heap_free(heap_ctrl* h, void* ptr);

void heap_resize(heap_ctrl* h, u64 new_size);