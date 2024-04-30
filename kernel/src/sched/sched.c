#include <sched/sched.h>
#include <sys/smp.h>
#include <lib/lock.h>

u64 sched_glob_id = 0;
atomic_lock sched_lock;

void sched_idle() {
  while (1) {
  }
}

void sched_wrapper(u64 func) {
  ((u64(*)())func)();
  this_cpu()->task_current->state = SCHED_DEAD;
  for (;;) {
    __asm__ volatile ("pause");
  }
}

void sched_init() {
  cpu_info* cpu = this_cpu();
  cpu->task_idx = -1;
  sched_new_task(sched_idle, cpu->lapic_id);
}

task_ctrl* sched_new_task(void* entry, u64 cpu) {
  lock(&sched_lock);
  
  cpu_info* c = get_cpu(cpu);

  task_ctrl* task = (task_ctrl*)kmalloc(sizeof(task_ctrl));
  memset(task, 0, sizeof(task_ctrl));

  char* stack = (char*)kmalloc(2 * PAGE_SIZE);
  task->ctx.rsp = (u64)(stack + (2 * PAGE_SIZE));
  task->ctx.rip = (u64)sched_wrapper;
  task->ctx.rdi = (u64)entry;
  task->ctx.cs  = 0x28;
  task->ctx.ss  = 0x30;
  task->ctx.rflags = 0x202;

  task->pm = vmm_new_pm();

  task->id = sched_glob_id++;
  task->cpu = cpu;
  task->stack_base = (u64)stack;
  task->state = SCHED_RUNNING;

  c->task_list[c->task_count++] = task;

  unlock(&sched_lock);

  return task;
}

task_ctrl* sched_get_next_task(cpu_info* c) {
  c->task_idx++;
  if (c->task_idx == c->task_count)
    c->task_idx = 0;
  task_ctrl* task = c->task_list[c->task_idx];
  while (task->state != SCHED_RUNNING) {
    c->task_idx++;
    if (c->task_idx == c->task_count)
      c->task_idx = 0;
    task = c->task_list[c->task_idx];
  }
  return task;
}

void sched_schedule(registers* r) {
  lapic_stop_timer();

  cpu_info* c = this_cpu();

  if (c->task_current != NULL)
    c->task_current->ctx = *r;
  
  c->task_current = sched_get_next_task(c);
  vmm_switch_pm(c->task_current->pm);
  *r = c->task_current->ctx;

  lapic_eoi();
  lapic_oneshot(0x32, 5);
}