#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_spawn(syscall_args a) {
  (void)a;
  //char* path = (char*)a.arg1;
  //u64 argc = (u64)a.arg2;
  //char** argv = (char**)a.arg3;

  u64 cpu = 1;
  cpu_info* c = get_cpu(cpu);
  u64 min_count = c->proc_list->count;
  for (u64 idx = 1 ; idx < smp_cpu_count; idx++) {
    c = get_cpu(idx);
    if (c->proc_list->count < min_count) {
      cpu = c->lapic_id;
      min_count = c->proc_list->count;
    }
  }

  //task_ctrl* task = sched_new_elf(path, cpu, argc, argv);
  //if (task == NULL) return 0;
  return 0;
}

u64 syscall_get_id(syscall_args a) {
  (void)a;
  return 0;
}