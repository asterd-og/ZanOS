#include <lib/lock.h>

void lock(atomic_lock* l) {
  while (__atomic_test_and_set(&l->locked, __ATOMIC_ACQUIRE)) {
    __asm__ volatile("pause");
  }
}

void unlock(atomic_lock* l) {
  __atomic_clear(&l->locked, __ATOMIC_RELEASE);
}