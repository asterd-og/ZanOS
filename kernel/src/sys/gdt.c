#include <sys/gdt.h>
#include <dev/lapic.h>

// Simple table containing all values for code and data (for 16, 32 and 64 bit).
u64 glob_table[] = {
  0x0000000000000000, // 0x00

  0x00009a000000ffff, // 0x08
  0x000093000000ffff,

  0x00cf9a000000ffff, // 0x10
  0x00cf93000000ffff,

  0x00af9b000000ffff, // 0x18
  0x00af93000000ffff,

  0x00affb000000ffff, // 0x20
  0x00aff3000000ffff,
};

tssr tss_list[256];

void gdt_init() {
  gdt_table table;

  memcpy(table.gdt_entries, glob_table, sizeof(glob_table));
  tss_entry* tss = &table.tss_entry;
  tssr* cpu_tss = &tss_list[lapic_get_id()];

  tss->limit  = (u64)(cpu_tss) + sizeof(*cpu_tss);
  tss->base   = (u16)cpu_tss;
  tss->base1  = (u8)((u64)cpu_tss >> 16);
  tss->base2  = (u8)((u64)cpu_tss >> 24);
  tss->base3  = (u32)((u64)cpu_tss >> 32);
  tss->flags  = 0x89;
  tss->flags1 = 0;
  tss->resv   = 0;

  gdtr gdt = (gdtr){
    .size = sizeof(gdt_table) - 1,
    .address = (u64)&table
  };

  gdt_flush(&gdt);
  u64 stack = 0;
  __asm__ volatile ("movq %%rsp, %0" : "=r"(stack));
  tss_list[lapic_get_id()].rsp[0] = stack + 0x4000;
}
