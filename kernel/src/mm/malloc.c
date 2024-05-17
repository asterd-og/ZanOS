#include <mm/malloc.h>
#include <mm/heap.h>
#include <sched/sched.h>
#include <sys/smp.h>

void* malloc(u64 size) {
  // TODO: Fix this
  return heap_alloc(this_cpu()->task_current->heap_area, size);
}

void free(void* ptr) {
  heap_free(this_cpu()->task_current->heap_area, ptr);
}