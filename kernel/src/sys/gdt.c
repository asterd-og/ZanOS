#include <sys/gdt.h>
#include <dev/lapic.h>

// Simple table containing all values for code and data (for 16, 32 and 64 bit).
u64 gdt_table[] = {
  0x0000000000000000,

  0x00009a000000ffff, // 16 bit code
  0x000093000000ffff, // data

  0x00cf9a000000ffff, // 32 bit
  0x00cf93000000ffff,

  0x00af9b000000ffff, // 64 bit
  0x00af93000000ffff,

  0x00affb000000ffff, // user mode
  0x00aff3000000ffff
};

void gdt_init() {
  gdtr gdt = (gdtr){
    .size = (sizeof(u64) * 9) - 1,
    .address = (u64)&gdt_table
  };

  __asm__ volatile ("lgdt %0\n\t" : : "m"(gdt) : "memory");
}
