#pragma once

#include <types.h>

typedef struct {
  u16 size;
  u64 address;
} __attribute__((packed)) gdtr;

typedef struct {
  u32 resv;
  u64 rsp[3];
  u64 resv1;
  u64 ist[7];
  u64 resv2;
  u16 resv3;
  u16 iopb;
} __attribute__((packed)) tssr; // Per CPU

extern tssr tss_list[256];

void gdt_init();