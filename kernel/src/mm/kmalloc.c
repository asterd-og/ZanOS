#include <mm/kmalloc.h>

heap* kernel_heap;

void kheap_init() {
  kernel_heap = heap_create();
}

void* kmalloc(u64 size) {
  void* ptr = heap_alloc(kernel_heap, size);
  return ptr;
}

void kfree(void* ptr) {
  heap_free(kernel_heap, ptr);
}