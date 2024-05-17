#pragma once

#include <types.h>
#include <fs/vfs.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct {
  vfs_node* vnode;
  u64 offset;
  u64 flags;
} file_descriptor;

file_descriptor fd_open(vfs_node* vnode);
i32 fprintf(int fd, char* fmt, ...);