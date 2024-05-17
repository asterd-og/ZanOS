#pragma once

#include <types.h>

// User functions for heap_alloc

void* malloc(u64 size);
void free(void* ptr);