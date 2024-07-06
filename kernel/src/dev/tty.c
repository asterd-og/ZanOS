#include <dev/tty.h>
#include <dev/dev.h>
#include <dev/char/serial.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>
#include <kernel.h>

vfs_node* tty_node;

// TODO: Actual tty!

i32 tty_write(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode;
  printf("%.*s", count, buffer);
  return count;
}

i32 tty_read(vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode; (void)buffer;
  return count;
}

u64 tty_poll(vfs_node* vnode) {
  tty* term = (tty*)vnode->obj;
  return term->flags;
}

void tty_init() {
  tty_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  tty_node->name = (char*)kmalloc(4);
  memcpy(tty_node->name, "tty", 4);
  tty_node->ino = 0;
  tty_node->write = tty_write;
  tty_node->read = tty_read;
  tty_node->readdir = 0;
  tty_node->finddir = 0;
  tty_node->poll = tty_poll;
  tty_node->size = 1;
  tty_node->type = VFS_DEVICE;

  tty* term = (tty*)kmalloc(sizeof(tty));
  term->buffer = (u8*)kmalloc(512);
  memset(term->buffer, 0, 512);
  term->buffer_write_idx = 0;
  term->buffer_read_idx = 0;
  term->buffer_size = 512;

  tty_node->obj = term;
  dev_add(tty_node);
}
