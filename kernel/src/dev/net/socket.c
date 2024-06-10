#include <dev/net/socket.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>
#include <dev/char/serial.h>
#include <sched/sched.h>
#include <sys/smp.h>

// TODO: Change sockets to use buffers instead of linked list
// Did that just to test some shit.

socket* sock_list[1024];
u64 sock_idx = 0;

int socket_read(vfs_node* node, u8* buffer, u32 count) {
  socket* sock = (socket*)node->obj;
  if (!sock) return -1;

  while (!(sock->flags & SOCK_MESSAGES))
    yield(); // Poll this socket until it's ready to be read from. Yield to save CPU time.

  if (sock->read_data == sock->data_head)
    sock->read_data = sock->read_data->next;
  socket_data* data = sock->read_data;

  usize size = (count > data->size ? data->size : count);
  memcpy(buffer, data->buffer, size);

  kfree(data->buffer);
  data->prev->next = data->next;
  data->next->prev = data->prev;
  kfree(data);

  sock->read_data = sock->read_data->next;

  sock->messages--;
  if (sock->messages == 0)
    sock->flags &= ~SOCK_MESSAGES;

  return size;
}

int socket_write(vfs_node* node, u8* buffer, u32 count) {
  socket* sock = (socket*)node->obj;
  if (!sock) return -1;

  while (sock->flags & SOCK_WRITING)
    yield(); // Poll this socket until it's ready to be written to!

  socket_data* data = (socket_data*)kmalloc(sizeof(socket_data));
  data->next = sock->data_head;
  data->prev = sock->data_head->prev;
  sock->data_head->prev->next = data;
  sock->data_head->prev = data;

  data->size = count;

  data->sender = this_cpu()->task_current->id;

  data->buffer = (u8*)kmalloc(count);
  memcpy(data->buffer, buffer, count);

  sock->messages++;
  sock->flags &= ~SOCK_WRITING;
  sock->flags |= SOCK_MESSAGES;
  return count;
}

socket* socket_open(task_ctrl* parent, u8 type, u64 buf_size, u64 max_conn) {
  socket* sock = (socket*)kmalloc(sizeof(socket));
  sock->parent = parent;
  sock->type = type;
  sock->buf_size = buf_size;
  sock->flags = 0;

  sock->data_head = (socket_data*)kmalloc(sizeof(socket_data));
  sock->data_head->next = sock->data_head;
  sock->data_head->prev = sock->data_head;

  sock->read_data = sock->data_head;
  sock->write_data = sock->data_head;

  sock->addrlen = 0;

  sock->max_conn = max_conn;
  sock->conn_count = 0;
  sock->conn_fds = (u64*)kmalloc(sock->max_conn * sizeof(u64));

  sock->messages = 0;
  sock->connected = 0;

  sock->conn_req_count = 0;

  sock_list[sock_idx++] = sock;
  return sock;
}

int socket_bind(socket* sock, char* address) {
  int addrlen = strlen(address);
  sock->address = (char*)kmalloc(addrlen + 1);
  memcpy(sock->address, address, addrlen + 1);
  sock->addrlen = addrlen;
  sock->connected = true;
  return 0;
}

vfs_node* socket_create(socket* sock) {
  vfs_node* node = (vfs_node*)kmalloc(sizeof(vfs_node));
  node->name = (char*)kmalloc(5);
  memcpy(node->name, "sock", 5);
  node->ino = 0;
  node->tid = this_cpu()->task_current->id;
  node->write = socket_write;
  node->read = socket_read;
  node->readdir = 0;
  node->finddir = 0;
  node->size = sock->buf_size;
  node->offset = 0;
  node->type = VFS_SOCKET;
  node->obj = sock;
  return node;
}

int socket_accept(socket* sock) {
  // If there are any connecting sockets, accept them and return their FDs
  if (!sock->conn_req_count) return 0;

  socket* from = sock->conn_req[sock->conn_req_count-1];
  vfs_node* node = socket_create(from);
  u64 fd = sock->parent->fd_idx++;
  sock->parent->fds[fd] = fd_open(node, FS_READ | FS_WRITE, fd);
  sock->conn_fds[sock->conn_count] = fd;
  from->connected = true;

  sock->conn_req_count--; sock->conn_count++;
  return fd;
}

int socket_connect(socket* from, char* address) {
  socket* sock = socket_find(address);
  if (!sock)
    return -1;
  if (sock->conn_count >= sock->max_conn || sock->conn_req_count >= SOCK_MAX_REQ_COUNT)
    return -1;

  sock->conn_req[sock->conn_req_count++] = from;
  while (!from->connected)
    yield(); // Wait until it has connected
  return 0;
}

int socket_get_ready(socket* sock) {
  if (!sock->conn_count) return -1;
  socket* client;
  for (u64 i = 0; i < sock->conn_count; i++) {
    client = (socket*)sock->parent->fds[sock->conn_fds[i]].vnode->obj;
    if (client->messages) {
      return sock->conn_fds[i];
    }
  }
  return -1;
}

socket* socket_find(char* address) {
  int addrlen = strlen(address);
  socket* sock = NULL;
  for (u64 i = 0; i < sock_idx; i++) {
    sock = sock_list[i];
    if (sock->addrlen > 0 && addrlen == sock->addrlen) {
      if (!memcmp(sock->address, address, addrlen))
        return sock;
    }
  }
  return NULL;
}

u64 socket_poll(socket* sock) {
  // For now, just return it's flags;
  return sock->flags;
}

u64 socket_msg_sender(socket* sock) {
  if (sock->read_data->next == sock->data_head) return 0;
  return sock->read_data->next->sender;
}
