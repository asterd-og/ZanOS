#pragma once

#include <types.h>

#define EV_KEY 0x1

typedef struct {
  u16 type; // key mod etc
  u16 value; // pressed released repeated
  u8 code;
} keyboard_event;

void keyboard_init();