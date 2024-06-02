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

#define SOCK_MAX_REQ_COUNT 0x10

typedef struct socket {
  task_ctrl* parent;
  u8 type;
  
  u64 buf_size;
  u64 flags;
  u64 messages;
  u8* buffer;
  
  char* address;
  int addrlen;

  u64 read_offset;
  u64 write_offset;
  
  u64 max_conn;
  u64 conn_count;
  u64* conn_fds;

  u64 conn_req_count;
  struct socket* conn_req[SOCK_MAX_REQ_COUNT];
} socket;

socket* socket_open(task_ctrl* parent, u8 type, u64 buf_size, u64 max_conn);
int socket_bind(socket* sock, char* address);
vfs_node* socket_create(socket* sock);

int socket_accept(socket* sock);
int socket_connect(socket* from, char* address);

int socket_get_ready(socket* sock);

socket* socket_find(char* address);