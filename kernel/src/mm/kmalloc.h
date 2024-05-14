#pragma once

#include <types.h>
#include <mm/heap.h>

extern heap* kernel_heap;

void kheap_init();
void* kmalloc(u64 size);
void kfree(void* ptr);