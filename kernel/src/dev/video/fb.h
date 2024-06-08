#pragma once

#include <types.h>

typedef struct {
  u32 width;
  u32 height;
  u32 pitch;
  u32 bpp;
} fb_info;

extern fb_info* fb;

void fb_init();