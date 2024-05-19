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
  for (u16 vec = 0; vec < 256; vec++)
    idt_set_entry(vec, idt_int_table[vec], (0x80 | (0x3 << 5) | 0xE));

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

void idt_set_entry(u8 vec, void* isr, u8 attr) {
  idt_entries[vec].low  = (u64)isr & 0xFFFF;
  idt_entries[vec].cs   = 0x28;
  idt_entries[vec].ist  = 0;
  idt_entries[vec].attr = attr;
  idt_entries[vec].mid  = ((u64)isr >> 16) & 0xFFFF;
  idt_entries[vec].high = ((u64)isr >> 32) & 0xFFFFFFFF;
  idt_entries[vec].resv = 0;
}

void irq_register(u8 vec, void* handler) {
  ioapic_redirect_irq(bsp_lapic_id, vec + 32, vec, false);
  irq_handlers[vec] = handler;
}

void irq_unregister(u8 vec) {
  irq_handlers[vec] = NULL;
}

void isr_handler(registers* r) {
  if (r->int_no == 0x80) {
    syscall_handler(r);
    return;
  }
  
  if (r->int_no == 0xff)
    return; // Spurious interrupt
  
  if (r->int_no == 14) {
    // Page fault
    if (this_cpu()->pm != vmm_kernel_pm) {
      sig_raise(SIGSEGV);
      return;
    }
  }
  
  __asm__ volatile ("cli");
  dprintf("isr_handler(): System fault! %s.\n", isr_errors[r->int_no]);
  for (;;) __asm__ volatile("hlt");
}

void irq_handler(registers* r) {
  void(*handler)(registers*);
  handler = irq_handlers[r->int_no - 32];

  if (handler != NULL)
    handler(r);
}