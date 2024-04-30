#pragma once

#include <types.h>

typedef struct {
  bool locked;
} atomic_lock;

void lock(atomic_lock* l);
void unlock(atomic_lock* l);