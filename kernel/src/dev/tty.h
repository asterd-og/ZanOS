#pragma once

#include <types.h>
#include <fs/vfs.h>
#include <lib/spinlock.h>

typedef struct {
  u8* buffer;
  u64 buffer_size;
  u64 buffer_write_idx;
  u64 buffer_read_idx;
  u64 flags;
  spinlock lock;
} tty;

extern vfs_node* tty_node;

void tty_init();