#pragma once

#include <types.h>
#include <sys/idt.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <fs/fs.h>
#include <lib/list.h>

enum {
  SCHED_STARTING,
  SCHED_RUNNING,
  SCHED_SLEEPING,
  SCHED_BLOCKED,
  SCHED_DEAD
};

enum {
  SCHED_IDLE,
  SCHED_KERNEL,
  SCHED_USER
};

typedef void(*signal_handler)(int);

typedef struct {
  u64 stack_base;   // gs:0
  u64 kernel_stack; // gs:8

  u64 stack_bottom;
  u64 kstack_bottom;

  u64 gs;

  registers ctx;

  u64 sleeping_time;

  u8 state;
  char fxsave[512] __attribute__((aligned(16)));

  pagemap* pm;
  heap* heap_area;

  signal_handler sigs[32];
} thread;

typedef struct process {
  u64 pid;
  u64 cpu;

  pagemap* pm;

  u8 type; // Idle, Kernel or User?

  vfs_node* current_dir;
  file_descriptor fds[256];
  u16 fd_idx;

  atomic_lock lock;

  char* name;

  list* threads;  // thread
  list* children; // process

  struct process* parent;

  u64 tidx; // thread index
  bool scheduled;
} process;

void sched_init();

process* sched_new_proc(char* name, void* entry, u8 type, u64 cpu);
thread* proc_add_thread(process* proc, void* entry, u64 cpu); // Adds a thread to the process

process* this_proc();
thread* this_thread();

//task_ctrl* sched_new_task(void* entry, u64 cpu, bool idle);

char** sched_create_argv(int argc, ...);
//task_ctrl* sched_new_elf(char* path, u64 cpu, int argc, char** argv);

void sched_schedule(registers* r);

void block();
//void unblock(task_ctrl* task);
void yield();
void sleep(u64 ms);

//void sched_block(task_ctrl* task, u8 reason);
//void sched_unblock(task_ctrl* task);
//void sched_kill(task_ctrl* task);
//task_ctrl* sched_get_task(u64 id);
void sched_exit(int status);