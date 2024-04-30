#include <sys/gdt.h>

// Simple table containing all values for code and data (for 16, 32 and 64 bit).
u64 gdt_table[] = {
  0x0000000000000000,

  0x00009a000000ffff,
  0x000093000000ffff,

  0x00cf9a000000ffff,
  0x00cf93000000ffff,

  0x00af9b000000ffff,
  0x00af93000000ffff,

  0x00affb000000ffff,
  0x00aff3000000ffff
};

void gdt_init() {
  gdtr gdt = (gdtr){
    .size = (sizeof(u64) * 9) - 1,
    .address = (u64)&gdt_table
  };

  __asm__ volatile ("lgdt %0\n\t" : : "m"(gdt) : "memory");
}
