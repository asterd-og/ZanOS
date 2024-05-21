#pragma once

#include <types.h>
#include <mm/pmm.h>

#define PTE_PRESENT (u64)1
#define PTE_WRITABLE (u64)2
#define PTE_USER (u64)4
#define PTE_NX (1ull << 63)

#define PTE_ADDR_MASK 0x000ffffffffff000
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define PTE_GET_FLAGS(VALUE) ((VALUE) & ~PTE_ADDR_MASK)

typedef uptr pagemap;

extern pagemap* vmm_kernel_pm;

extern symbol text_start_ld;
extern symbol text_end_ld;

extern symbol rodata_start_ld;
extern symbol rodata_end_ld;

extern symbol data_start_ld;
extern symbol data_end_ld;

void vmm_init();

pagemap* vmm_new_pm();

void vmm_switch_pm_nocpu(pagemap* pm);
void vmm_switch_pm(pagemap* pm);

void vmm_map(pagemap* pm, uptr vaddr, uptr paddr, u64 flags);
void vmm_map_user(pagemap* pm, uptr vaddr, uptr paddr, u64 flags);
void vmm_unmap(pagemap* pm, uptr vaddr);
void vmm_map_range(pagemap* pm, uptr vaddr, uptr paddr, u64 pages, u64 flags);
void vmm_map_user_range(pagemap* pm, uptr vaddr, uptr paddr, u64 pages, u64 flags);

void* vmm_alloc(u64 pages, u64 flags);
void vmm_free(void* ptr, u64 pages);