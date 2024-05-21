#pragma once

#include <types.h>

#define IA32_GS_MSR 0xC0000101
#define IA32_GS_KERNEL_MSR 0xC0000102

u64 rdmsr(u32 msr);
void wrmsr(u32 msr, u64 val);

u64 read_cpu_gs();
void write_cpu_gs(u64 value);
u64 read_kernel_gs();
void write_kernel_gs(u64 value);