#pragma once

#include <types.h>

typedef struct list {
  u64 count;
  u64 limit;
  u64* ptr;
} list;

list* list_create();
void list_add(list* head, void* v);
void* list_get(list* head, u64 idx);