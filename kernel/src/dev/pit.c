#include <dev/pit.h>
#include <dev/lapic.h>
#include <sys/ports.h>
#include <sched/sched.h>

u64 pit_ticks = 0;
thread* pit_sleeping_thread = NULL;

void pit_handler(registers* r) {
  (void)r;
  pit_ticks++;
  if (sched_sleep_list != NULL) {
    /*for (u64 i = 0; i < sched_sleep_list->count; i++) {
      pit_sleeping_thread = (thread*)list_iterate(sched_sleep_list, true);
      if (pit_sleeping_thread->sleeping_time-- == 0) {
        pit_sleeping_thread->state = SCHED_RUNNING;
        list_remove(sched_sleep_list, i);
        // Wake up this thread, AKA Jump to it
        unblock(pit_sleeping_thread);
      }
    }*/
  }
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