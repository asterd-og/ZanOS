#pragma once

#include <types.h>
#include <fs/vfs.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define FS_READ 0x1000
#define FS_WRITE 0x2000
#define FS_CREATE 0x4000

typedef struct {
  vfs_node* vnode;
  u64 offset;
  u16 mode;
  u16 fd_num;
} file_descriptor;

file_descriptor fd_open(vfs_node* vnode, u16 mode, u16 fd_num);
i32 fprintf(int fd, char* fmt, ...);