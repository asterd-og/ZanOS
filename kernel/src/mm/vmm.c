#include <mm/vmm.h>
#include <limine.h>
#include <lib/libc.h>
#include <dev/char/serial.h>
#include <sys/smp.h>

struct limine_kernel_address_request kernel_address_request = {
  .id = LIMINE_KERNEL_ADDRESS_REQUEST,
  .revision = 0
};

pagemap* vmm_kernel_pm = NULL;

void vmm_init() {
  vmm_kernel_pm = (pagemap*)HIGHER_HALF(pmm_alloc(1));
  memset(vmm_kernel_pm, 0, PAGE_SIZE);

  uptr phys_base = kernel_address_request.response->physical_base;
  uptr virt_base = kernel_address_request.response->virtual_base;

  uptr text_start = ALIGN_DOWN((uptr)text_start_ld, PAGE_SIZE);
  uptr text_end = ALIGN_UP((uptr)text_end_ld, PAGE_SIZE);
  uptr rodata_start = ALIGN_DOWN((uptr)rodata_start_ld, PAGE_SIZE);
  uptr rodata_end = ALIGN_UP((uptr)rodata_end_ld, PAGE_SIZE);
  uptr data_start = ALIGN_DOWN((uptr)data_start_ld, PAGE_SIZE);
  uptr data_end = ALIGN_UP((uptr)data_end_ld, PAGE_SIZE);

  for (uptr text = text_start; text < text_end; text += PAGE_SIZE)
    vmm_map(vmm_kernel_pm, text, text - virt_base + phys_base, PTE_PRESENT);
  for (uptr rodata = rodata_start; rodata < rodata_end; rodata += PAGE_SIZE)
    vmm_map(vmm_kernel_pm, rodata, rodata - virt_base + phys_base, PTE_PRESENT | PTE_NX);
  for (uptr data = data_start; data < data_end; data += PAGE_SIZE)
    vmm_map(vmm_kernel_pm, data, data - virt_base + phys_base, PTE_PRESENT | PTE_WRITABLE | PTE_NX);
  for (uptr gb4 = 0; gb4 < 0x100000000; gb4 += PAGE_SIZE) {
    vmm_map(vmm_kernel_pm, gb4, gb4, PTE_PRESENT | PTE_WRITABLE);
    vmm_map(vmm_kernel_pm, (uptr)HIGHER_HALF(gb4), gb4, PTE_PRESENT | PTE_WRITABLE);
  }

  vmm_switch_pm_nocpu(vmm_kernel_pm);
  dprintf("vmm_init(): VMM Initialised. Kernel's page map located at %lx.\n", (u64)vmm_kernel_pm);
}

uptr* vmm_get_next_lvl(uptr* lvl, uptr entry, u64 flags, bool alloc) {
  if (lvl[entry] & PTE_PRESENT)
    return HIGHER_HALF(PTE_GET_ADDR(lvl[entry]));
  if (alloc) {
    uptr* pml = (uptr*)HIGHER_HALF(pmm_alloc(1));
    memset(pml, 0, PAGE_SIZE);
    lvl[entry] = (uptr)PHYSICAL(pml) | flags;
    return pml;
  }
  return NULL;
}

pagemap* vmm_new_pm() {
  pagemap* pm = (pagemap*)HIGHER_HALF(pmm_alloc(1));
  memset(pm, 0, PAGE_SIZE);
  for (usize i = 256; i < 512; i++)
    pm[i] = vmm_kernel_pm[i];
  return pm;
}

void vmm_switch_pm_nocpu(pagemap* pm) {
  __asm__ volatile ("mov %0, %%cr3" : : "r"((u64)PHYSICAL(pm)) : "memory");
}

void vmm_switch_pm(pagemap* pm) {
  __asm__ volatile ("mov %0, %%cr3" : : "r"((u64)PHYSICAL(pm)) : "memory");
  this_cpu()->pm = pm;
}

void vmm_map(pagemap* pm, uptr vaddr, uptr paddr, u64 flags) {
  uptr pml1_entry = (vaddr >> 12) & 0x1ff;
  uptr pml2_entry = (vaddr >> 21) & 0x1ff;
  uptr pml3_entry = (vaddr >> 30) & 0x1ff;
  uptr pml4_entry = (vaddr >> 39) & 0x1ff;

  uptr* pml3 = vmm_get_next_lvl(pm, pml4_entry, PTE_PRESENT | PTE_WRITABLE, true);       // pml4[pml4Entry] = pml3
  uptr* pml2 = vmm_get_next_lvl(pml3, pml3_entry, PTE_PRESENT | PTE_WRITABLE, true);     // pml3[pml3Entry] = pml2
  uptr* pml1 = vmm_get_next_lvl(pml2, pml2_entry, PTE_PRESENT | PTE_WRITABLE, true);     // pml2[pml2Entry] = pml1

  pml1[pml1_entry] = paddr | flags;
}

void vmm_map_user(pagemap* pm, uptr vaddr, uptr paddr, u64 flags) {
  uptr pml1_entry = (vaddr >> 12) & 0x1ff;
  uptr pml2_entry = (vaddr >> 21) & 0x1ff;
  uptr pml3_entry = (vaddr >> 30) & 0x1ff;
  uptr pml4_entry = (vaddr >> 39) & 0x1ff;

  uptr* pml3 = vmm_get_next_lvl(pm, pml4_entry, flags, true);       // pml4[pml4Entry] = pml3
  uptr* pml2 = vmm_get_next_lvl(pml3, pml3_entry, flags, true);     // pml3[pml3Entry] = pml2
  uptr* pml1 = vmm_get_next_lvl(pml2, pml2_entry, flags, true);     // pml2[pml2Entry] = pml1

  pml1[pml1_entry] = paddr | flags;
}

void vmm_unmap(pagemap* pm, uptr vaddr) {
  uptr pml1_entry = (vaddr >> 12) & 0x1ff;
  uptr pml2_entry = (vaddr >> 21) & 0x1ff;
  uptr pml3_entry = (vaddr >> 30) & 0x1ff;
  uptr pml4_entry = (vaddr >> 39) & 0x1ff;

  uptr* pml3 = vmm_get_next_lvl(pm, pml4_entry, 0, false);
  if (pml3 == NULL) return;
  uptr* pml2 = vmm_get_next_lvl(pml3, pml3_entry, 0, false);
  if (pml2 == NULL) return;
  uptr* pml1 = vmm_get_next_lvl(pml2, pml2_entry, 0, false);
  if (pml1 == NULL) return;
  pml1[pml1_entry] = 0;
  __asm__ volatile ("invlpg (%0)" : : "b"(vaddr) : "memory");
}

void vmm_map_range(pagemap* pm, uptr vaddr, uptr paddr, u64 pages, u64 flags) {
  for (u64 i = 0; i < pages; i++)
    vmm_map(pm, vaddr + (i * PAGE_SIZE), paddr + (i * PAGE_SIZE), flags);
}

void vmm_map_user_range(pagemap* pm, uptr vaddr, uptr paddr, u64 pages, u64 flags) {
  for (u64 i = 0; i < pages; i++)
    vmm_map_user(pm, vaddr + (i * PAGE_SIZE), paddr + (i * PAGE_SIZE), flags);
}

void* vmm_alloc(u64 pages, u64 flags) {
  void* pg = pmm_alloc(pages);
  if (!pg) return NULL;
  for (u64 p = 0; p < pages; p++)
    vmm_map(this_cpu()->pm, (uptr)pg + (p * PAGE_SIZE), (uptr)pg + (p * PAGE_SIZE), flags);
  return pg;
}

void vmm_free(void* ptr, u64 pages) {
  if (!ptr) return;
  pmm_free(ptr, pages);
  for (u64 p = 0; p < pages; p++)
    vmm_unmap(this_cpu()->pm, (uptr)ptr + (p * PAGE_SIZE));
}