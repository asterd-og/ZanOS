#pragma once

#include <types.h>
#include <dev/dev.h>

#define EV_KEY 0x1

extern vfs_node* kb_node;

typedef struct {
  u16 type; // key mod etc
  u16 value; // pressed released repeated
  u8 code;
} keyboard_event;

void keyboard_init();