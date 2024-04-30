#pragma once

#include <types.h>
#include <sys/idt.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>

enum {
  SCHED_STARTING,
  SCHED_RUNNING,
  SCHED_DEAD
};

typedef struct task_ctrl {
  registers ctx;
  pagemap* pm;
  u64 id;
  u64 cpu;
  u64 stack_base;
  u8 state;
} task_ctrl;

void sched_init();

task_ctrl* sched_new_task(void* entry, u64 cpu);
void sched_schedule();