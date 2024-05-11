#pragma once

#include <types.h>
#include <sys/idt.h>

#define PIT_FREQ 1000

extern u64 pit_ticks;

void pit_init();

void pit_sleep(u64 ms);