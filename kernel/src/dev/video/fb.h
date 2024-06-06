#pragma once

#include <types.h>

typedef struct {
  u32 width;
  u32 height;
  u32 pitch;
  u32 bpp;
} fb_info;

void fb_init();