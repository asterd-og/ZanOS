#include <mm/heap.h>
#include <lib/libc.h>
#include <dev/char/serial.h>

heap* heap_create() {
  heap* h = (heap*)HIGHER_HALF(pmm_alloc(1));
  h->block_head = (heap_block*)pmm_alloc(1);
  h->block_head->magic = HEAP_MAGIC;
  h->block_head->next = h->block_head->prev = h->block_head;
  h->block_head->size = 0;
  h->block_head->state = 1;
  return h;
}

void* heap_alloc(heap* h, u64 size) {
  u64 pages = DIV_ROUND_UP(sizeof(heap_block) + size, PAGE_SIZE);
  u8* buf = (u8*)pmm_alloc(pages);
  heap_block* block = (heap_block*)buf;
  block->magic = HEAP_MAGIC;
  block->size = size;
  block->state = 1;
  block->prev = h->block_head->prev;
  block->next = h->block_head;
  h->block_head->prev->next = block;
  h->block_head->prev = block;
  return buf + sizeof(heap_block);
}

void heap_free(heap* h, void* ptr) {
  heap_block* block = (heap_block*)(ptr - sizeof(heap_block));
  if (block->magic != HEAP_MAGIC) {
    dprintf("heap_free(): Invalid magic at pointer %lx.\n", ptr);
    return;
  }
  block->prev->next = block->next;
  block->next->prev = block->prev;
  u64 pages = DIV_ROUND_UP(sizeof(heap_block) + block->size, PAGE_SIZE);
  u8* buf = (u8*)(ptr - sizeof(heap_block));
  pmm_free(buf, pages);
}