#include <mm/heap.h>
#include <lib/libc.h>
#include <dev/char/serial.h>

u64 glob_id = 0;

heap_pool* heap_create_pool(heap_ctrl* h, u64 initial_size) {
  heap_pool* pool = HIGHER_HALF(pmm_alloc(1));
  if ((u64)pool == hhdm_offset) {
    dprintf("heap_create_pool(): Couldn't allocate structure for the pool.\n");
    return NULL;
  }
  pool->next = pool; pool->prev = pool;
  pool->pool = pmm_alloc(initial_size / PAGE_SIZE);
  if (pool->pool == NULL) {
    dprintf("heap_create_pool(): Couldn't allocate the pool.\n");
    return NULL;
  }
  pool->size = initial_size;
  pool->used = false;
  pool->last_ptr = 0;
  pool->id = h->pool_count++;
  pool->free_size = pool->size;
  return pool;
}

heap_ctrl* heap_create(u64 size) {
  heap_ctrl* h = HIGHER_HALF(pmm_alloc(1));
  if ((u64)h == hhdm_offset) {
    dprintf("heap_init(): Couldn't allocate structure for a new heap.\n");
    return NULL;
  }
  h->pool_count = 0;
  u64 aligned_size = DIV_ROUND_UP(size, PAGE_SIZE);
  h->pool_head = heap_create_pool(h, aligned_size * PAGE_SIZE);
  if (h->pool_head == NULL) {
    dprintf("heap_init(): Couldn't allocate heap's pool.\n");
    return NULL;
  }
  h->size = aligned_size * PAGE_SIZE;
  h->id = glob_id++;
  dprintf("heap_init(): Created a new heap with size %ld at %lx.\n", aligned_size * PAGE_SIZE, (u64)h);
  return h;
}

heap_pool* heap_find_next_pool(heap_ctrl* h, u64 size) {
  heap_pool* p = h->pool_head;
  while (p->free_size < size) {
    if (p->next == h->pool_head) return NULL;
    p = p->next;
  }
  return p;
}

u64 heap_find_next_free(heap_pool* p, u64 size) {
  if (p->last_ptr + (14 + size) < p->size)
    return p->last_ptr;
  p->last_ptr = 0;
  while (p->last_ptr < p->size) {
    u32 magic = *(u32*)(p->pool + p->last_ptr);
    if (magic != HEAP_MAGIC)
      return p->last_ptr;
    p->last_ptr += 6;
    u64 size = *(u64*)(p->pool + p->last_ptr);
    if (size == 0) {
      p->last_ptr -= 6;
      return p->last_ptr;
    }
    p->last_ptr += 8 + size;
  }
  return -1;
}

void* heap_alloc(heap_ctrl* h, u64 size) {
  if (size == 0 || h == NULL) return NULL;
  heap_pool* p = heap_find_next_pool(h, size + 14);
  if (p == NULL)
    return NULL;
  u64 data_off = heap_find_next_free(p, size);
  if (data_off == -1)
    return NULL;
  u8* data = p->pool + data_off;
  *(u32*)(data) = HEAP_MAGIC;
  *(u16*)(data + 4) = p->id;
  *(u64*)(data + 6) = size;
  p->last_ptr += 14;
  void* ptr = data + p->last_ptr;
  p->last_ptr += size;
  if (p->last_ptr + 14 >= p->size)
    p->used = true;
  p->free_size -= size + 14;
  return ptr;
}

void heap_free(heap_ctrl* h, void* ptr) {
  u64 size = *(u64*)(ptr - 8);
  u16 pool_id = *(u16*)(ptr - 10);
  u32 magic = *(u32*)(ptr - 14);
  if (magic != HEAP_MAGIC) {
    dprintf("heap_free(): Invalid magic at pointer %lx on heap %ld.\n", (u64)ptr, h->id);
    return;
  }
  heap_pool* p = h->pool_head;
  for (u16 i = 0; i < pool_id; i++) {
    p = p->next;
  }
  p->free_size += size + 14;
  if (p->used)
    p->used = false;
  *(u64*)(ptr - 8) = 0; // Magic is set, but size is 0, our heap will understand that as free
  memset(ptr, 0, size);
}

void heap_resize(heap_ctrl* h, u64 new_size) {
  heap_pool* p = heap_create_pool(h, new_size);
  if (p == NULL) {
    dprintf("heap_resize(): Couldn't allocate pool for resize of heap %ld.\n", h->id);
    return;
  }
  p->prev = h->pool_head->prev;
  p->next = h->pool_head;
  h->pool_head->prev->next = p;
  h->pool_head->prev = p;
  dprintf("heap_resize(): Resized heap %ld successfully.\n", h->id);
}