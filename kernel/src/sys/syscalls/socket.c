#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <dev/net/socket.h>

u64 syscall_sock_new(syscall_args a) {
  u64 type = (u64)a.arg1;
  u64 buf_size = (u64)a.arg2;
  char* address = (char*)a.arg3;
  if (!buf_size)
    buf_size = 8192; // 8 kb

  if (type != SOCK_LOCAL && type != SOCK_NET)
    return (u64)-1;

  task_ctrl* task = this_cpu()->task_current;
  
  socket* sock = socket_open((u8)type, buf_size, address);
  if (!sock) return (u64)-1;
  vfs_node* node = socket_create(sock);
  if (!node) return (u64)-1;

  u64 fd = task->fd_idx;
  task->fds[fd] = fd_open(node, FS_READ | FS_WRITE, fd);
  task->fd_idx++;

  return fd;
}

// Finds and opens a socket, returns it's fd.
u64 syscall_sock_connect(syscall_args a) {
  char* address = (char*)a.arg1;
  if (!address)
    return (u64)-1;
  task_ctrl* task = this_cpu()->task_current;
  
  socket* sock = socket_find(address);
  if (!sock)
    return (u64)-1;

  vfs_node* node = socket_create(sock);
  if (!node) return (u64)-1;

  u64 fd = task->fd_idx;
  task->fds[fd] = fd_open(node, FS_READ | FS_WRITE, fd);
  task->fd_idx++;

  return fd;
}