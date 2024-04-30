#pragma once

#include <types.h>

inline void bitmap_set(u8* bitmap, u64 bit) {
    bitmap[bit / 8] |= 1 << (bit % 8);
}

inline void bitmap_clear(u8* bitmap, u64 bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

inline u8 bitmap_get(u8* bitmap, u64 bit) {
    return bitmap[bit / 8] & 1 << (bit % 8);
}
