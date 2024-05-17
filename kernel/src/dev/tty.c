#include <dev/tty.h>
#include <dev/dev.h>
#include <mm/kmalloc.h>
#include <kernel.h>

vfs_node* tty_node;

i32 tty_write(struct vfs_node* vnode, u8* buffer, u32 count) {
  flanterm_write(ft_ctx, buffer, count);
  return count;
}

void tty_init() {
  tty_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  tty_node->name = (char*)kmalloc(4);
  memcpy(tty_node->name, "tty", 4);
  tty_node->ino = 0;
  tty_node->write = tty_write;
  tty_node->read = 0;
  tty_node->readdir = 0;
  tty_node->finddir = 0;
  tty_node->size = 1;
  tty_node->type = VFS_DEVICE;
  dev_add(tty_node);
}