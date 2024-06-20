#pragma once

#include <types.h>
#include <limine.h>
#include <dev/lapic.h>
#include <dev/char/serial.h>
#include <sched/sched.h>
#include <mm/vmm.h>
#include <lib/list.h>

extern u32 bsp_lapic_id;
extern u64 smp_cpu_count;

typedef struct {
  u64 lapic_id;
  pagemap* pm;

  list* proc_list;

  process* proc;
  thread* thread;

  u64 proc_idx;

  process* idle_proc;

  atomic_lock sched_lock;
} cpu_info;

void smp_init();
void smp_get_bsp();

cpu_info* get_cpu(u64 lapic_id);
cpu_info* this_cpu();