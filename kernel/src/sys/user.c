#include <sys/user.h>
#include <sys/cpu.h>

void user_init() {
  u64 star = rdmsr(IA32_STAR);
  star |= ((u64)0x28 << 32); // kernel seg base
  star |= ((u64)0x38 << 48); // user seg base
  wrmsr(IA32_STAR, star);
  wrmsr(IA32_CSTAR + 1, 0x202); // Clear the rflags bit
}