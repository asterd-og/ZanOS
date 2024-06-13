#include <sys/idt.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <dev/char/serial.h>
#include <dev/lapic.h>
#include <dev/ioapic.h>
#include <sched/sched.h>
#include <sched/signal.h>

__attribute__((aligned(0x10))) static idt_entry idt_entries[256];
static idtr idt;
extern void* idt_int_table[];

void* irq_handlers[256] = {};

static const char* isr_errors[32] = {
  "Division by zero",
  "Debug",
  "Non-maskable interrupt",
  "Breakpoint",
  "Detected overflow",
  "Out-of-bounds",
  "Invalid opcode",
  "No coprocessor",
  "Double fault",
  "Coprocessor segment overrun",
  "Bad TSS",
  "Segment not present",
  "Stack fault",
  "General protection fault",
  "Page fault",
  "Unknown interrupt",
  "Coprocessor fault",
  "Alignment check",
  "Machine check",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved"
};

void idt_init() {
  for (u16 vec = 0; vec < 256; vec++) {
    idt_set_entry(vec, idt_int_table[vec], 0x8E, (vec == 0x80 ? 0 : 0));
  }

  idt = (idtr){
    .size   = (u16)sizeof(idt_entry) * 256 - 1,
    .offset = (u64)idt_entries
  };

  __asm__ volatile ("lidt %0" : : "m"(idt) : "memory");
  __asm__ volatile ("sti");
}

void idt_reinit() {
  __asm__ ("lidt %0" : : "m"(idt) : "memory");
  __asm__ ("sti");
}

void idt_set_entry(u8 vec, void* isr, u8 type, u8 dpl) {
  idt_entries[vec].low  = (u64)isr & 0xFFFF;
  idt_entries[vec].cs   = 0x28;
  idt_entries[vec].ist  = 0;
  idt_entries[vec].type = type;
  idt_entries[vec].dpl  = dpl;
  idt_entries[vec].p    = 1;
  idt_entries[vec].mid  = ((u64)isr >> 16) & 0xFFFF;
  idt_entries[vec].high = ((u64)isr >> 32) & 0xFFFFFFFF;
  idt_entries[vec].resv = 0;
}

void irq_register(u8 vec, void* handler) {
  if (vec < 15)
    ioapic_redirect_irq(bsp_lapic_id, vec + 32, vec, false);
  irq_handlers[vec] = handler;
}

void irq_unregister(u8 vec) {
  irq_handlers[vec] = NULL;
}

void isr_handler(registers* r) {
  if (r->int_no == 0xff)
    return; // Spurious interrupt
  
  if (r->int_no == 14) {
    // Page fault
    if (this_cpu()->pm != vmm_kernel_pm) {
      u64 cr2;
      __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
      dprintf("isr_handler(): Task segmentation fault. RIP %lx. RSP %lx CR2 %lx PM %lx task %lu\n", r->rip, r->rsp, cr2, this_cpu()->pm, this_cpu()->task_current->id, r->err_code);
      if (!(r->err_code & PTE_PRESENT)) dprintf("isr_handler(): Page was not present, ");
      else dprintf("isr_handler(): Page was present, ");
      if (!(r->err_code & PTE_WRITABLE)) dprintf("was not writable, ");
      else dprintf("was writable, ");
      if (!(r->err_code & PTE_USER)) dprintf("and was not user.\n");
      else dprintf("and was user.\n");
      sig_raise(SIGSEGV);
      return;
    }
  }
  
  __asm__ volatile ("cli");
  dprintf("isr_handler(): System fault! %s. RIP: %llx. CS: %x SS: %x\n", isr_errors[r->int_no], r->rip, r->cs, r->ss);
  for (;;) __asm__ volatile("hlt");
}

void irq_handler(registers* r) {
  void(*handler)(registers*);
  handler = irq_handlers[r->int_no - 32];

  if (handler != NULL)
    handler(r);
}