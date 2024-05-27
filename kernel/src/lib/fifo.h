#pragma once

#include <types.h>
#include <lib/lock.h>

typedef struct {
  atomic_lock lock;
  u64 cap;
  u64** data;
  u64 idx;
  u64 count;
} fifo;

fifo* fifo_create(u64 cap);
void fifo_push(fifo* fifo, u64* val);
u64* fifo_pop(fifo* fifo);