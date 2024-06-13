#include <mm/kmalloc.h>

heap* kernel_heap;

void kheap_init() {
  kernel_heap = heap_create(vmm_kernel_pm);
}

void* kmalloc(u64 size) {
  return heap_alloc(kernel_heap, size);
}

void kfree(void* ptr) {
  heap_free(kernel_heap, ptr);
}

void* krealloc(void* ptr, u64 size) {
  return heap_realloc(kernel_heap, ptr, size);
}