#pragma once

#include <types.h>
#include <lib/lock.h>

typedef struct list_item {
  void* val;
  struct list_item* next;
  struct list_item* prev;
} list_item;

typedef struct list {
  atomic_lock lock;
  u64 count;
  list_item* head;
  list_item* iterator;
} list;

list* list_create();
void list_add(list* l, void* v);
void list_remove(list* l, list_item* item);
void* list_iterate(list* l, bool wrap);
list_item* list_find(list* l, void* v);
