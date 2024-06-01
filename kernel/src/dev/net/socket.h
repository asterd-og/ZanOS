#pragma once

#include <types.h>
#include <fs/vfs.h>
#include <lib/semaphore.h>
#include <sched/sched.h>

#define SOCK_LOCAL 0x1
#define SOCK_NET 0x2
#define SOCK_SIZE 0x8192

#define SOCK_WRITING 0x1
#define SOCK_UPDATE 0x2
#define SOCK_CLEARED 0x3

typedef struct {
  u8 type;
  u64 buf_size;
  u64 flags;
  u64 messages;
  u8* buffer;
  char* address;
  int addrlen;
  u64 read_offset;
  u64 write_offset;
} socket;

socket* socket_open(u8 type, u64 buf_size, char* address);
vfs_node* socket_create(socket* sock);

int socket_connect(task_ctrl* task, socket* sock);

socket* socket_find(char* address);