#include <dev/lapic.h>
#include <dev/char/serial.h>
#include <dev/pit.h>
#include <sys/cpu.h>
#include <sys/smp.h>
#include <mm/pmm.h>

u64 apic_ticks = 0;

void lapic_init() {
  lapic_write(0xf0, lapic_read(0xf0) | 0x100);
  dprintf("lapic_init(): LAPIC Initialised.\n");
}

void lapic_stop_timer() {
  // We do this to avoid overlapping oneshots
  lapic_write(LAPIC_TIMER_INITCNT, 0);
  lapic_write(LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);
}

void lapic_oneshot(u8 vec, u64 ms) {
  lapic_stop_timer();
  lapic_write(LAPIC_TIMER_DIV, 0);
  lapic_write(LAPIC_TIMER_LVT, vec);
  lapic_write(LAPIC_TIMER_INITCNT, apic_ticks * ms);
}

void lapic_calibrate_timer() {
  lapic_stop_timer();
  lapic_write(LAPIC_TIMER_DIV, 0);
  lapic_write(LAPIC_TIMER_LVT, 0);
  lapic_write(LAPIC_TIMER_INITCNT, 0xFFFFFFFF);
  pit_sleep(1); // 10 ms
  lapic_write(LAPIC_TIMER_LVT, LAPIC_TIMER_DISABLE);
  u32 ticks = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURCNT);
  apic_ticks = ticks;
  lapic_stop_timer();
}

void lapic_write(u32 reg, u32 val) {
  *((volatile u32*)(HIGHER_HALF(0xfee00000) + reg)) = val;
}

u32 lapic_read(u32 reg) {
  return *((volatile u32*)(HIGHER_HALF(0xfee00000) + reg));
}

void lapic_eoi() {
  lapic_write((u8)0xb0, 0x0);
}

void lapic_ipi(u32 id, u32 dat) {
  lapic_write(LAPIC_ICRHI, id << LAPIC_ICDESTSHIFT);
  lapic_write(LAPIC_ICRLO, dat);

  while ((lapic_read(LAPIC_ICRLO) & LAPIC_ICPEND) != 0)
    ;
}

void lapic_send_all_int(u32 id, u32 vec) {
  lapic_ipi(id, vec | LAPIC_ICRAIS);
}

void lapic_send_others_int(u32 id, u32 vec) {
  lapic_ipi(id, vec | LAPIC_ICRAES);
}

void lapic_init_cpu(u32 id) {
  lapic_ipi(id, (u32)(LAPIC_ICINI | LAPIC_ICPHYS | LAPIC_ICASSR |
    LAPIC_ICEDGE | LAPIC_ICSHRTHND));
}

void lapic_start_cpu(u32 id, u32 vec) {
  lapic_ipi(id, vec | LAPIC_ICSTR | LAPIC_ICPHYS | LAPIC_ICASSR |
    LAPIC_ICEDGE | LAPIC_ICSHRTHND);
}

u32 lapic_get_id() {
  return lapic_read(0x0020) >> LAPIC_ICDESTSHIFT;
}

void lapic_set_base(u64 lapic) {
  wrmsr(0x800, (lapic & 0xfffff000ULL) | 0x800);
}

u64 lapic_get_base() {
  return rdmsr(0x1bU) & 0xfffff000ULL;
}