#include <dev/pit.h>
#include <dev/lapic.h>
#include <sys/ports.h>

u64 pit_ticks = 0;

void pit_handler(registers* r) {
  (void)r;
  pit_ticks++;
  lapic_eoi();
}

void pit_init() {
  outb(0x43, 0x36);
  u16 div = (u16)(1193180 / PIT_FREQ);
  outb(0x40, (u8)div);
  outb(0x40, (u8)(div >> 8));
  irq_register(0, pit_handler);
}

void pit_sleep(u64 ms) {
  u64 start = pit_ticks;
  while (pit_ticks - start < ms) {
    __asm__ volatile ("nop");
  }
}