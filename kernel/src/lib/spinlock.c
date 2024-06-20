#include <lib/spinlock.h>
#include <sched/sched.h>

void acquire(spinlock* spin) {
  while (atomic_flag_test_and_set_explicit(spin, memory_order_acquire))
    yield();
}

void release(spinlock* spin) {
  atomic_flag_clear_explicit(spin, memory_order_release);
}