#pragma once

#include <types.h>
#include <limine.h>
#include <dev/lapic.h>
#include <dev/char/serial.h>
#include <sched/sched.h>

extern u32 bsp_lapic_id;
extern u64 smp_cpu_count;

typedef struct {
  u64 lapic_id;

  u64 task_count;
  u64 task_idx;
  task_ctrl* task_list[256];
  task_ctrl* task_current;
} cpu_info;

void smp_init();
void smp_get_bsp();

cpu_info* get_cpu(u64 lapic_id);
cpu_info* this_cpu();