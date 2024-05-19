#include <mm/malloc.h>
#include <mm/heap.h>
#include <sched/sched.h>
#include <sys/smp.h>

void* malloc(u64 size) {
  void* ret = heap_alloc(this_cpu()->task_current->heap_area, size);
  return ret;
}

void free(void* ptr) {
  heap_free(this_cpu()->task_current->heap_area, ptr);
}

void* realloc(void* ptr, u64 size) {
  return heap_realloc(this_cpu()->task_current->heap_area, ptr, size);
}