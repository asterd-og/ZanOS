#include <lib/elf.h>
#include <lib/libc.h>
#include <sys/smp.h>

u64 elf_load(u8* img, pagemap* pm) {
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
      size_t seg_size = seg_end - seg_start;
      void* seg = pmm_alloc(seg_size / PAGE_SIZE);
      vmm_map_user_range(pm, seg_start, (uptr)seg, seg_size / PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
      vmm_create_region(pm, seg_start, (uptr)seg, seg_size / PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
      pagemap* old_pm = this_cpu()->pm;
      vmm_switch_pm(pm);
      memcpy((void*)phdr->vaddr, (void*)img + phdr->offset, phdr->file_size);
      vmm_switch_pm(old_pm);
    }
  }
  return hdr->entry;
}
