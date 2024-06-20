#include <dev/char/serial.h>
#include <dev/dev.h>
#include <lib/lock.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>

vfs_node* serial_node;

void serial_write_char(char c, void* extra) {
  (void)extra;
  outb(0xe9, c);
}

void dprintf(const char* fmt, ...) {
  static atomic_lock l;
  lock(&l);
  va_list args;
  va_start(args, fmt);
  vfctprintf(serial_write_char, NULL, fmt, args);
  va_end(args);
  unlock(&l);
}

i32 serial_write(vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode;
  dprintf("%s", buffer);
  return count;
}

void serial_init() {
  serial_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  serial_node->name = (char*)kmalloc(7);
  memcpy(serial_node->name, "serial", 7);
  serial_node->ino = 0;
  serial_node->write = serial_write;
  serial_node->read = 0;
  serial_node->readdir = 0;
  serial_node->finddir = 0;
  serial_node->size = 1;
  serial_node->type = VFS_DEVICE;
  dev_add(serial_node);
}