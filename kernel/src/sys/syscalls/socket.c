#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <dev/net/socket.h>

u64 syscall_sock_new(syscall_args a) {
  u64 type = (u64)a.arg1;
  u64 buf_size = (u64)a.arg2;
  u64 max_conn = (u64)a.arg3;
  if (!buf_size)
    buf_size = 8192; // 8 kb

  if (type != SOCK_LOCAL && type != SOCK_NET)
    return (u64)-1;

  task_ctrl* task = this_cpu()->task_current;
  
  socket* sock = socket_open(task, (u8)type, buf_size, max_conn);
  if (!sock) return (u64)-1;
  vfs_node* node = socket_create(sock);
  if (!node) return (u64)-1;

  u64 fd = task->fd_idx;
  task->fds[fd] = fd_open(node, FS_READ | FS_WRITE, fd);
  task->fd_idx++;

  return fd;
}

u64 syscall_sock_bind(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  char* address = (char*)a.arg2;
  if (!address) return (u64)-1;
  task_ctrl* task = this_cpu()->task_current;

  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;
  int ret = socket_bind(sock, address);

  return (u64)ret;
}

// Connects to a sock, enters it's to connect queue and waits.
u64 syscall_sock_connect(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  char* address = (char*)a.arg2;
  if (!address)
    return (u64)-1;
  task_ctrl* task = this_cpu()->task_current;

  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;
  int ret = socket_connect(sock, address);

  return (u64)ret;
}

// Accepts any socket on the to connect queue, opens it and returns it's fd
u64 syscall_sock_accept(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  task_ctrl* task = this_cpu()->task_current;

  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;

  int ret = socket_accept(sock);

  return (u64)ret;
}

// Returns the fd num of the socket ready to be read from, -1 means no sockets are ready.
u64 syscall_sock_get_ready(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  task_ctrl* task = this_cpu()->task_current;

  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;

  int ret = socket_get_ready(sock);

  return (u64)ret;
}

u64 syscall_sock_poll(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  task_ctrl* task = this_cpu()->task_current;

  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;

  return (u64)socket_poll(sock);
}

u64 syscall_sock_msg_sender(syscall_args a) {
  u64 sockfd = (u64)a.arg1;
  task_ctrl* task = this_cpu()->task_current;
  
  if (!task->fds[sockfd].vnode) return (u64)-1;

  socket* sock = (socket*)task->fds[sockfd].vnode->obj;

  return socket_msg_sender(sock);
}
