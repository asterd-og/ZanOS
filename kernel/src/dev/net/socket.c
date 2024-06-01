#include <dev/net/socket.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>
#include <dev/char/serial.h>
#include <sched/sched.h>

socket* sock_list[1024];
u64 sock_idx = 0;

int socket_read(vfs_node* node, u8* buffer, u32 count) {
  socket* sock = (socket*)node->obj;
  while (!sock->messages)
    yield(); // yield until it's ready to be read from.
  
  if (count > sock->buf_size || node->offset + count > sock->buf_size)
    return -1;
  memcpy(buffer, sock->buffer + sock->read_offset, count);
  sock->read_offset += count;
  if (sock->read_offset >= sock->buf_size) {
    // If the offset overflows, clear the buffer.
    sock->read_offset = 0;
    memset(sock->buffer, 0, sock->buf_size);
    sock->flags |= SOCK_CLEARED;
  }
  sock->messages--;
  return count;
}

int socket_write(vfs_node* node, u8* buffer, u32 count) {
  socket* sock = (socket*)node->obj;

  while (sock->flags & SOCK_WRITING)
    yield(); // It's currently being written to by another task, wait.

  sock->flags |= SOCK_WRITING;

  if (count > sock->buf_size || sock->write_offset + count > sock->buf_size) {
    if (count > sock->buf_size)
      return -1;
    // Count is within the buf size, just set the write off to 0 again
    // let's sure hope that read has been called already, or it'll be overwritten!
    sock->write_offset = 0;
  }
  memcpy(sock->buffer + sock->write_offset, buffer, count);
  sock->write_offset += count;
  sock->messages++;
  sock->flags &= ~SOCK_WRITING;
  return count;
}

socket* socket_open(u8 type, u64 buf_size, char* address) {
  socket* sock = (socket*)kmalloc(sizeof(socket));
  sock->type = type;
  sock->buf_size = buf_size;
  sock->flags = 0;
  sock->buffer = (u8*)kmalloc(buf_size);
  memset(sock->buffer, 0, buf_size);
  
  int addrlen = strlen(address);
  sock->address = (char*)kmalloc(addrlen + 1);
  memcpy(sock->address, address, addrlen + 1);
  sock->addrlen = addrlen;

  sock_list[sock_idx++] = sock;
  return sock;
}

vfs_node* socket_create(socket* sock) {
  vfs_node* node = (vfs_node*)kmalloc(sizeof(vfs_node));
  node->name = (char*)kmalloc(5);
  memcpy(node->name, "sock", 5);
  node->ino = 0;
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

socket* socket_find(char* address) {
  int addrlen = strlen(address);
  socket* sock = NULL;
  for (u64 i = 0; i < sock_idx; i++) {
    sock = sock_list[i];
    if (addrlen == sock->addrlen) {
      if (!memcmp(sock->address, address, addrlen))
        return sock;
    }
  }
  return NULL;
}