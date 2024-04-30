#pragma once

#include <types.h>
#include <kernel.h>

#define PAGE_SIZE 4096

#define DIV_ROUND_UP(x, y) (x + (y - 1)) / y
#define ALIGN_UP(x, y) DIV_ROUND_UP(x, y) * y
#define ALIGN_DOWN(x, y) (x / y) * y

#define HIGHER_HALF(ptr) ((void*)((u64)ptr) + hhdm_offset)
#define PHYSICAL(ptr) ((void*)((u64)ptr) - hhdm_offset)

void pmm_init();

void* pmm_alloc(usize n);
void pmm_free(void* ptr, usize n);
