#pragma once

#include <types.h>
#include <mm/vmm.h>

typedef struct {
  unsigned char ident[16];
  u16 type;
  u16 isa;
  u32 elf_version;
  u64 entry;
  u64 phoff;
  u64 shoff;
  u32 flags;
  u16 hdr_size;
  u16 entry_ph_size;
  u16 entry_ph_count;
  u16 entry_sh_size;
  u16 entry_sh_count;
  u16 sh_names;
} elf_header;

typedef struct {
  u32 type;
  u32 flags;
  u64 offset;
  u64 vaddr;
  u64 paddr;
  u64 file_size;
  u64 mem_size;
  u64 align;
} elf_ph;

u64 elf_load(char* img, pagemap* pm);