#include <lib/fifo.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>

fifo* fifo_create(u64 cap, u64 item_size) {
  fifo* f = (fifo*)kmalloc(sizeof(fifo));
  f->cap = cap;
  f->data = (void**)kmalloc(cap * item_size);
  memset(f->data, 0, cap * item_size);
  f->item_size = item_size;
  f->idx = 0;
  f->count = 0;
  return f;
}

void fifo_push(fifo* f, void* val) {
  lock(&f->lock);
  if (f->count == f->cap) {
    unlock(&f->lock);
    return;
  }
  memcpy(f->data + (f->count * f->item_size), val, f->item_size);
  f->count++;
  unlock(&f->lock);
}

void fifo_pop(fifo* f, void* buffer) {
  lock(&f->lock);
  if (f->count == 0) {
    unlock(&f->lock);
    memset(buffer, 0, f->item_size);
    return;
  }
  memcpy(buffer, f->data + (f->idx * f->item_size), f->item_size);
  f->idx++;
  if (f->idx == f->count) {
    f->idx = 0; f->count = 0;
    memset(f->data, 0, f->item_size * f->cap);
  }
  unlock(&f->lock);
}