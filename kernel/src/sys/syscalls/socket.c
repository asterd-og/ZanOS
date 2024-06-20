#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>

u64 syscall_sock_new(syscall_args a) {
  (void)a;
  return 0;
}

u64 syscall_sock_bind(syscall_args a) {
  (void)a;
  return 0;
}

// Connects to a sock, enters it's to connect queue and waits.
u64 syscall_sock_connect(syscall_args a) {
  (void)a;
  return 0;
}

// Accepts any socket on the to connect queue, opens it and returns it's fd
u64 syscall_sock_accept(syscall_args a) {
  (void)a;
  return 0;
}

// Returns the fd num of the socket ready to be read from, -1 means no sockets are ready.
u64 syscall_sock_get_ready(syscall_args a) {
  (void)a;
  return 0;
}

u64 syscall_sock_msg_sender(syscall_args a) {
  (void)a;
  return 0;
}
