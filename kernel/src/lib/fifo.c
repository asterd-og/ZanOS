#include <lib/fifo.h>
#include <mm/kmalloc.h>

fifo* fifo_create(u64 cap) {
  fifo* f = (fifo*)kmalloc(sizeof(fifo));
  f->cap = cap;
  f->data = (u64**)kmalloc(cap * sizeof(u64*));
  f->idx = 0;
  f->count = 0;
  return f;
}

void fifo_push(fifo* f, u64* val) {
  lock(&f->lock);
  if (f->count == f->cap) {
    unlock(&f->lock);
    return;
  }
  f->data[f->count++] = val;
  unlock(&f->lock);
}

u64* fifo_pop(fifo* f) {
  lock(&f->lock);
  if (f->count == 0) {
    unlock(&f->lock);
    return NULL;
  }
  u64* val = f->data[f->idx++];
  if (f->idx == f->count) {
    f->idx = 0; f->count = 0;
  } else {
    f->idx++;
  }
  unlock(&f->lock);
  return val;
}