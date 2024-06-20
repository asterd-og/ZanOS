#pragma once

#include <types.h>
#include <stdatomic.h>

typedef atomic_flag spinlock;

void acquire(spinlock* spin);
void release(spinlock* spin);