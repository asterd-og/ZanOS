#pragma once

#include <types.h>

typedef struct {
  u16 low;
  u16 cs;
  u8  ist;
  u8  attr;
  u16 mid;
  u32 high;
  u32 resv;
} __attribute__((packed)) idt_entry;

typedef struct {
  u16 size;
  u64 offset;
} __attribute__((packed)) idtr;

typedef struct {
  u64 r15;
  u64 r14;
  u64 r13;
  u64 r12;
  u64 r11;
  u64 r10;
  u64 r9;
  u64 r8;
  u64 rdi;
  u64 rsi;
  u64 rbp;
  u64 rbx;
  u64 rdx;
  u64 rcx;
  u64 rax;
  u64 int_no;
  u64 err_code;
  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
} __attribute__((packed)) registers;

void idt_init();
void idt_reinit();

void idt_switch_to_apic();

void idt_set_entry(u8 vec, void* isr);

void irq_register(u8 vec, void* handler);
void irq_unregister(u8 vec);