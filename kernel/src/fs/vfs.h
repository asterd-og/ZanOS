#pragma once

#include <types.h>

typedef struct vfs {
  u32 id;
  void* object; // ext2_fs or something else
  int(*read)(struct vfs* v, const char* path, u8* buffer, u32 offset, u32 count);
} vfs;

void vfs_init();
int vfs_read(const char* path, u8* buffer, u32 offset, u32 count);