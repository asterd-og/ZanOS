#include <sys/smp.h>
#include <dev/char/serial.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <mm/vmm.h>
#include <mm/heap.h>

u32 bsp_lapic_id = 0;
u64 smp_cpu_count = 0;
u64 smp_cpu_started = 0;
cpu_info* smp_cpu_list[128];

struct limine_smp_request smp_request = {
  .id = LIMINE_SMP_REQUEST,
  .revision = 0
};

void smp_init_cpu(struct limine_smp_info* smp_info) {
  gdt_init();
  idt_reinit();
  vmm_switch_pm(vmm_kernel_pm);

  while (smp_cpu_started < smp_info->lapic_id - 1)
    __asm__ volatile ("pause");
  
  cpu_info* c = (cpu_info*)kmalloc(sizeof(cpu_info));
  memset(c, 0, sizeof(cpu_info));
  c->lapic_id = smp_info->lapic_id;
  c->pm = vmm_kernel_pm;

  smp_cpu_list[smp_info->lapic_id] = c;

  lapic_init();
  sched_init();

  dprintf("smp_init_cpu(): CPU %ld started.\n", smp_info->lapic_id);
  smp_cpu_started++;

  while (true) {
    __asm__ volatile ("hlt");
  }
}

void smp_init() {
  struct limine_smp_response* smp_response = smp_request.response;

  bsp_lapic_id = smp_response->bsp_lapic_id;
  smp_cpu_count = smp_response->cpu_count;
  dprintf("smp_init(): bsp_lapic_id: %d.\n", bsp_lapic_id);

  cpu_info* c = (cpu_info*)kmalloc(sizeof(cpu_info));
  memset(c, 0, sizeof(cpu_info));
  c->pm = vmm_kernel_pm;

  smp_cpu_list[0] = c;

  for (u64 i = 0; i < smp_cpu_count; i++)
    smp_response->cpus[i]->goto_address = smp_init_cpu;
  
  while (smp_cpu_started < smp_cpu_count - 1)
    __asm__ volatile ("nop");
}

cpu_info* get_cpu(u64 lapic_id) {
  return smp_cpu_list[lapic_id];
}

cpu_info* this_cpu() {
  return smp_cpu_list[lapic_get_id()];
}