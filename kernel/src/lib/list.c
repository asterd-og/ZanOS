#include <lib/list.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>
#include <dev/char/serial.h>

list* list_create() {
  list* l = (list*)kmalloc(sizeof(list));
  l->head = (list_item*)kmalloc(sizeof(list_item));
  l->head->next = l->head;
  l->head->prev = l->head;
  l->iterator = l->head;
  l->count = 0;
  unlock(&l->lock);
  return l;
}

void list_add(list* l, void* val) {
  lock(&l->lock);
  list_item* item = (list_item*)kmalloc(sizeof(list_item));
  if (!item) {
    unlock(&l->lock);
    return;
  }
  item->val = val;
  item->prev = l->head->prev;
  item->next = l->head;
  l->head->prev->next = item;
  l->head->prev = item;
  l->count++;
  unlock(&l->lock);
}

void list_remove(list* l, list_item* item) {
  lock(&l->lock);
  item->next->prev = item->prev;
  item->prev->next = item->next;
  l->count--;
  unlock(&l->lock);
}

void* list_iterate(list* l, bool wrap) {
  if (l->count == 0) return NULL;
  lock(&l->lock);
  l->iterator = l->iterator->next;
  if (l->iterator == l->head) {
    if (wrap) {
      l->iterator = l->head->next;
    } else {
      unlock(&l->lock);
      return NULL;
    }
  }
  unlock(&l->lock);
  return l->iterator->val;
}

list_item* list_find(list* l, void* v) {
  if (l->count == 0) return NULL;
  lock(&l->lock);
  for (list_item* item = l->head->next; item != l->head; item = item->next) {
    if (item->val == v) {
      unlock(&l->lock);
      return item;
    }
  }
  unlock(&l->lock);
  return NULL;
}

