#pragma once

#include <types.h>

void bitmap_set(u8* bitmap, u64 bit) {
    bitmap[bit / 8] |= 1 << (bit % 8);
}

void bitmap_clear(u8* bitmap, u64 bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

bool bitmap_get(u8* bitmap, u64 bit) {
    return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}