#pragma once

#include <types.h>
#include <sys/idt.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <fs/fs.h>
#include <lib/list.h>

#define SCHED_STACK_SIZE (4 * PAGE_SIZE)

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

struct process;

typedef struct {
  u64 stack_base;   // gs:0
  u64 kernel_stack; // gs:8

  u64 stack_bottom;
  u64 kstack_bottom;

  struct process* parent;
  u64 idx; // idx in proc thread list

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
  u64 idx; // index inside the cpu proc list

  pagemap* pm;

  u8 type; // Idle, Kernel or User?

  vfs_node* current_dir;
  file_descriptor fds[256];
  u16 fd_idx;

  atomic_lock lock;

  char* name;

  list* threads;  // thread

  struct process* parent;

  u64 tidx; // thread index
  bool scheduled;
} process;

void sched_init();

extern list* sched_sleep_list;

process* sched_new_proc(char* name, u8 type, u64 cpu, bool child);
thread* proc_add_thread(process* proc, void* entry, bool fork); // Adds a thread to the process
thread* proc_add_elf_thread(process* proc, char* path);

process* this_proc();
thread* this_thread();

char** sched_create_argv(int argc, ...);
void sched_schedule(registers* r);

void block();
void unblock(thread* t);
void yield();
void sleep(u64 ms);

void sched_exit(int status);