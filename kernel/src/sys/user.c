#include <sys/user.h>
#include <sys/cpu.h>

void syscall_entry();

void user_init() {
  u64 efer = rdmsr(IA32_EFER);
  efer |= (1 << 0);
  wrmsr(IA32_EFER, efer);
  u64 star = 0;
  star |= ((u64)0x28 << 32); // kernel cs
  star |= ((u64)0x33 << 48); // user cs (it loads 0x30 + 16 for cs, which is 0x40 and + 8 for ss 0x38)
  wrmsr(IA32_STAR, star);
  wrmsr(IA32_LSTAR, (u64)syscall_entry);
  wrmsr(IA32_CSTAR, 0);
  wrmsr(IA32_CSTAR+1, 0x200);
}