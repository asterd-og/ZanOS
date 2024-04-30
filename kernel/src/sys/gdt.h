#pragma once

#include <types.h>

typedef struct {
    u16 size;
    u64 address;
} __attribute__((packed)) gdtr;

void gdt_init();