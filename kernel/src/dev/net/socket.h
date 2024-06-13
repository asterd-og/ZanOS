#pragma once

#include <types.h>
#include <fs/vfs.h>
#include <lib/fifo.h>
#include <sched/sched.h>

#define SOCK_LOCAL 0x1
#define SOCK_NET 0x2
#define SOCK_SIZE 8192

#define SOCK_WRITING 0x1
#define SOCK_UPDATE 0x2
#define SOCK_CLEARED 0x4
#define SOCK_MESSAGES 0x6

#define SOCK_MAX_REQ_COUNT 0x10

typedef struct socket_message {
  u64 sender; // Server? client? contains sender's TID.
  u8* buffer;
  usize size;
} socket_message;

typedef struct socket {
  task_ctrl* parent;
  u8 type;
  
  u64 buf_size;
  u64 flags;
  u64 messages;

  bool connected;
  
  char* address;
  int addrlen;

  fifo* buffer;
  socket_message* temp_msg;
  
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

u64 socket_poll(socket* sock);
u64 socket_msg_sender(socket* sock);
