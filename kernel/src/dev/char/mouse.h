#pragma once

#include <types.h>

extern u32 mouse_x;
extern u32 mouse_y;

extern bool mouse_left_pressed;
extern bool mouse_right_pressed;

extern bool mouse_moved;

typedef struct {
  u32 x;
  u32 y;
  bool left_button;
  bool right_button;
} mouse_event;

void mouse_init();