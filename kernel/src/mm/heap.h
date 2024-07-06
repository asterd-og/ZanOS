#pragma once

#include <types.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/lock.h>

#define HEAP_MAGIC 0xdeadbeef

typedef struct heap_block {
  struct heap_block* next;
  struct heap_block* prev;
  u8 state;
  u32 magic;
  u64 size;
} heap_block;

typedef struct {
  atomic_lock hl;
  heap_block* block_head;
  pagemap* pm;
} heap;

heap* heap_create(pagemap* pm);

void* heap_alloc(heap* h, u64 size);
void heap_free(heap* h, void* ptr);
void* heap_realloc(heap* h, void* ptr, u64 size);

uptr heap_get_allocation_paddr(heap* h, uptr ptr);

void heap_clone(heap* h, heap* clone);
void heap_destroy(heap* h);
