#pragma once

#include <types.h>
#include <sys/idt.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <fs/fs.h>

enum {
  SCHED_STARTING,
  SCHED_RUNNING,
  SCHED_SLEEPING,
  SCHED_BLOCKED,
  SCHED_DEAD
};

typedef void(*signal_handler)(int);

typedef struct task_ctrl {
  registers ctx;
  pagemap* pm;
  bool user;
  u64 id;
  u64 cpu;
  u64 stack_base;
  u64 sleeping_time;
  u8 state;
  int exit_status;
  heap* heap_area;
  vfs_node* current_dir;
  file_descriptor fds[256];
  u16 fd_idx;
  signal_handler sigs[32];

  u64 gs_base; // To be switched with kernel stack
  u64 kernel_gs;
} task_ctrl;

void sched_init();

task_ctrl* sched_new_task(void* entry, u64 cpu, bool user);

char** sched_create_argv(int argc, ...);
task_ctrl* sched_new_elf(char* path, u64 cpu, int argc, char** argv, bool user);

void sched_schedule(registers* r);

void sleep(u64 ms);

void sched_block(task_ctrl* task, u8 reason);
void sched_unblock(task_ctrl* task);
void sched_kill(task_ctrl* task);
task_ctrl* sched_get_task(u64 id);
void sched_exit(int status);