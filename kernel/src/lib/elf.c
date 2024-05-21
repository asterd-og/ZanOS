#include <lib/elf.h>
#include <lib/libc.h>

u64 elf_load(char* img, pagemap* pm) {
  elf_header* hdr = (elf_header*)img;

  if (hdr->ident[0] != 0x7f || hdr->ident[1] != 'E' ||
    hdr->ident[2] != 'L' || hdr->ident[3] != 'F')
      return -1;

  if (hdr->type != 2)
    return -1;
  
  elf_ph* phdr = (elf_ph*)(img + hdr->phoff);

  for (u16 i = 0; i < hdr->entry_ph_count; i++, phdr++) {
    if (phdr->type == 1) {
      // Elf load
      uptr seg_start = ALIGN_DOWN(phdr->vaddr, PAGE_SIZE);
      uptr seg_end = ALIGN_UP(seg_start + phdr->mem_size, PAGE_SIZE);
      void* seg = pmm_alloc(DIV_ROUND_UP(phdr->mem_size, PAGE_SIZE));
      u64 j = 0;
      for (uptr vaddr = seg_start; vaddr <= seg_end; vaddr += PAGE_SIZE) {
        vmm_map_user(pm, vaddr, (uptr)seg + (j * PAGE_SIZE), PTE_PRESENT | PTE_WRITABLE | PTE_USER);
        j++;
      }
      vmm_switch_pm(pm);
      memcpy((void*)phdr->vaddr, (void*)img + phdr->offset, phdr->file_size);
      memset((void*)(phdr->vaddr + phdr->file_size), 0, phdr->mem_size - phdr->file_size);
      vmm_switch_pm(vmm_kernel_pm);
    }
  }
  return hdr->entry;
}