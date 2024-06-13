#pragma once

#include <types.h>
#include <lib/lock.h>

typedef struct {
  atomic_lock lock;
  u64 cap;
  void** data;
  u64 item_size;
  u64 idx;
  u64 count;
} fifo;

fifo* fifo_create(u64 cap, u64 item_size);
void fifo_push(fifo* fifo, void* val);
void fifo_pop(fifo* fifo, void* buffer);
void fifo_get(fifo* fifo, void* buffer);