#pragma once

#include <types.h>
#include <sys/idt.h>

#define PIT_FREQ 100

extern u64 pit_ticks;

void pit_init();

void pit_sleep(u64 ms); // Should be divisible by 10 (100 HZ, every tick = 10 ms)