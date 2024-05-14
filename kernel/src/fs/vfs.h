#pragma once

#include <types.h>

#define VFS_FILE 0x1
#define VFS_DIRECTORY 0x2

typedef struct {
  char* name;
  u32 ino;
} vfs_dirent;

typedef struct vfs_node {
  char* name;
  u32 perms;
  u32 type;
  u32 size;
  u32 ino;
  u32(*read)(struct vfs_node* vnode, u32 offset, u32 count, u8* buffer);
  vfs_dirent*(*readdir)(struct vfs_node* vnode, u32 index);
  struct vfs_node*(*finddir)(struct vfs_node* vnode, char* path);
} vfs_node;

extern vfs_node* vfs_root; // "/" path

void vfs_init();
u32 vfs_read(struct vfs_node* vnode, u32 offset, u32 count, u8* buffer);
vfs_dirent* vfs_readdir(struct vfs_node* vnode, u32 index);
vfs_node* vfs_finddir(struct vfs_node* vnode, char* path);