#include <sys/cpu.h>
#include <cpuid.h>

u64 rdmsr(u32 msr) {
  u32 low;
  u32 high;
  __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}

void wrmsr(u32 msr, u64 val) {
  __asm__ volatile ("wrmsr" : : "a"((u32)val), "d"((u32)(val >> 32)), "c"(msr));
}

u64 read_cpu_gs() {
  return rdmsr(IA32_GS_MSR);
}

void write_cpu_gs(u64 value) {
  wrmsr(IA32_GS_MSR, value);
}

u64 read_kernel_gs() {
  return rdmsr(IA32_GS_KERNEL_MSR);
}

void write_kernel_gs(u64 value) {
  wrmsr(IA32_GS_KERNEL_MSR, value);
}

void sse_enable() {
  u64 cr0;
  u64 cr4;
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0) : : "memory");
  cr0 &= ~((u64)1 << 2);
  cr0 |= (u64)1 << 1;
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
  __asm__ volatile("mov %%cr4, %0" :"=r"(cr4) : : "memory");
  cr4 |= (u64)3 << 9;
  __asm__ volatile("mov %0, %%cr4" : : "r"(cr4) : "memory");
}

u64 fpu_init() {
  u32 eax, unused, edx;
  __get_cpuid(1, &eax, &unused, &unused, &edx);
  if (!(edx & (1 << 0)))
    return 1; // CPU Doesnt support FPU.

  u64 cr0;
  __asm__ volatile("mov %%cr0, %0" : "=r"(cr0) : : "memory");
  cr0 &= ~(1 << 3); // Clear TS bit
  cr0 &= ~(1 << 2); // Clear EM bit
  __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
  __asm__ volatile("fninit");
  return 0;
}