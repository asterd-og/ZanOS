#pragma once

#include <types.h>
#include <sys/idt.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>

enum {
  SCHED_STARTING,
  SCHED_RUNNING,
  SCHED_SLEEPING,
  SCHED_BLOCKED,
  SCHED_DEAD
};

typedef struct task_ctrl {
  registers ctx;
  pagemap* pm;
  u64 id;
  u64 cpu;
  u64 stack_base;
  u64 sleeping_time;
  u8 state;
} task_ctrl;

void sched_init();

task_ctrl* sched_new_task(void* entry, u64 cpu);

void sched_schedule(registers* r);

void sleep(u64 ms);

void sched_block(task_ctrl* task, u8 reason);
void sched_unblock(task_ctrl* task);