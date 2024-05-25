#pragma once

#include <types.h>

#define VFS_FILE 0x1
#define VFS_DIRECTORY 0x2
#define VFS_DEVICE 0x3

#define VFS_DESTROY 0x1 // Destroy perm

typedef struct {
  char* name;
  u32 ino;
} vfs_dirent;

typedef struct vfs_node {
  char* name;
  struct vfs_node* parent;
  bool open;
  u32 perms;
  u32 type;
  u32 size;
  u32 ino;
  i32(*read)(struct vfs_node* vnode, u8* buffer, u32 count);
  i32(*write)(struct vfs_node* vnode, u8* buffer, u32 count);
  vfs_dirent*(*readdir)(struct vfs_node* vnode, u32 index);
  struct vfs_node*(*finddir)(struct vfs_node* vnode, char* path);
} vfs_node;

typedef struct {
  vfs_node* node;
  u32 current_index;
} DIR; // For syscalls

extern vfs_node* vfs_root; // "/" path

void vfs_init();
i32 vfs_write(vfs_node* vnode, u8* buffer, u32 count);
i32 vfs_read(vfs_node* vnode, u8* buffer, u32 count);
vfs_dirent* vfs_readdir(vfs_node* vnode, u32 index);
vfs_node* vfs_finddir(vfs_node* vnode, char* path);
vfs_node* vfs_open(vfs_node* vnode, char* path); // traverse directories
void vfs_destroy(vfs_node* vnode);
