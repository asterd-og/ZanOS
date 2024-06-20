#include <lib/list.h>
#include <mm/kmalloc.h>

list* list_create() {
  list* l = (list*)kmalloc(sizeof(list));
  l->ptr = (u64*)kmalloc(sizeof(u64) * 32);
  l->limit = 32;
  l->count = 0;
  return l;
}

void list_add(list* l, void* val) {
  if (l->count > l->limit) {
    l->ptr = krealloc(l->ptr, l->limit + 32);
    l->limit += 32;
  }
  l->ptr[l->count++] = (u64)val;
}

void list_remove(list* l, u64 idx) {
  l->ptr[idx] = 0;
  l->count--;
}

void* list_get(list* l, u64 idx) {
  if (idx > l->count) return NULL;
  return (void*)l->ptr[idx];
}