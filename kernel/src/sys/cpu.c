#include <sys/cpu.h>

u64 rdmsr(u32 msr) {
  u32 low;
  u32 high;
  __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}

void wrmsr(u32 msr, u64 val) {
  __asm__ volatile ("wrmsr" : : "a"((u32)val), "d"((u32)(val >> 32)), "c"(msr));
}