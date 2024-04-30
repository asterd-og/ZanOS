#include <mm/pmm.h>
#include <limine.h>
#include <lib/bitmap.h>
#include <lib/libc.h>
#include <dev/char/serial.h>

u8* pmm_bitmap = NULL;
u64 pmm_free_pages = 0;
u64 pmm_used_pages = 0;
u64 pmm_total_pages = 0;
u64 pmm_last_page = 0;

struct limine_memmap_request memmap_request = {
  .id = LIMINE_MEMMAP_REQUEST,
  .revision = 0
};

struct limine_memmap_response* pmm_memmap = NULL;

void pmm_init() {
  pmm_memmap = memmap_request.response;
  struct limine_memmap_entry** entries = pmm_memmap->entries;
  
  u64 higher_address = 0;
  u64 top_address = 0;

  struct limine_memmap_entry* entry;

  for (u64 i = 0; i < pmm_memmap->entry_count; i++) {
    entry = entries[i];
    if (entry->type != LIMINE_MEMMAP_USABLE)
      continue;
    top_address = entry->base + entry->length;
    if (top_address > higher_address)
      higher_address = top_address;
  }

  pmm_total_pages = higher_address / PAGE_SIZE;
  u64 bitmap_size = ALIGN_UP(pmm_total_pages / 8, PAGE_SIZE);

  for (u64 i = 0; i < pmm_memmap->entry_count; i++) {
    entry = entries[i];
    if (entry->type != LIMINE_MEMMAP_USABLE || entry->length < bitmap_size) continue;
    pmm_bitmap = (u8*)HIGHER_HALF(entry->base);
    memset(pmm_bitmap, 0xFF, bitmap_size);
    entry->base += bitmap_size;
    entry->length -= bitmap_size;
    break;
  }

  for (u64 i = 0; i < pmm_memmap->entry_count; i++) {
    entry = entries[i];
    if (entry->type != LIMINE_MEMMAP_USABLE) continue;
    for (u64 j = 0; j < entry->length; j += PAGE_SIZE)
      bitmap_clear(pmm_bitmap, (entry->base + j) / PAGE_SIZE);
  }
  dprintf("pmm_init(): PMM Initialised at %lx with bitmap size of %ld.\n", (u64)pmm_bitmap, bitmap_size);
}

void* pmm_alloc(usize n) {
  u64 pages = 0;
  while (pages < n) {
    if (pmm_last_page == pmm_total_pages) {
      dprintf("pmm_alloc(): Ran out of memory.\n");
      return NULL;
    }
    if (bitmap_get(pmm_bitmap, pmm_last_page + pages) == 0)
      pages++;
    else {
      pmm_last_page++;
      pages = 0;
    }
  }
  for (u64 i = 0; i < n; i++)
    bitmap_set(pmm_bitmap, pmm_last_page + i);
  pmm_last_page += pages;
  return (void*)((pmm_last_page - n) * PAGE_SIZE); // Return the start of the pages
}

void pmm_free(void* ptr, usize n) {
  u64 idx = (u64)ptr / PAGE_SIZE;
  for (u64 i = 0; i < n; i++)
    bitmap_clear(pmm_bitmap, idx + i);
  pmm_last_page = idx;
}
