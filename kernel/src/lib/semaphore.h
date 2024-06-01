#pragma once

#include <types.h>
#include <sched/sched.h>

typedef struct {
  i32 inc;
  
  task_ctrl* blocked_tasks[256];
  u8 idx;
  u8 fidx; // fifo idx
} semaphore;

void semaphore_init(semaphore* s, i32 init_state);

void wait(semaphore* s);
void signal(semaphore* s);