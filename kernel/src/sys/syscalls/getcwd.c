#include <sys/syscall.h>
#include <sys/smp.h>
#include <sched/sched.h>
#include <mm/malloc.h>

u64 syscall_getcwd(syscall_args a) {
  char* buf = (char*)a.arg1;
  size_t size = (size_t)a.arg2;
  task_ctrl* task = this_cpu()->task_current;
  if (!buf) {
    if (!size) size = 256;
    buf = (char*)malloc(size);
  }
  memset(buf, 0, size);
  vfs_node nodes[128];
  u32 i = 1;
  vfs_node* current = task->current_dir;
  while (current != NULL) {
    nodes[i++] = *current;
    current = current->parent;
  }
  u32 pos = 0;
  u32 len = 0;
  for (u32 j = i - 1; j > 0; j--) {
    len = strlen(nodes[j].name);
    memcpy(buf + pos, nodes[j].name, len);
    pos += len;
    if (j > 1 && j != i -1) {
      buf[pos] = '/';
      pos++;
    }
  }
  return (u64)buf;
}