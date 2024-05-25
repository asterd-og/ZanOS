#include <kernel.h>
#include <dev/video/fb.h>
#include <dev/dev.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>

vfs_node* fb_node;

i32 fb_write(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode;
  memcpy(framebuffer->address, buffer, count);
  return count;
}

void fb_init() {
  fb_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  fb_node->name = (char*)kmalloc(4);
  memcpy(fb_node->name, "fb0", 4);
  fb_node->ino = 0;
  fb_node->write = fb_write;
  fb_node->read = 0;
  fb_node->readdir = 0;
  fb_node->finddir = 0;
  fb_node->size = 1;
  fb_node->type = VFS_DEVICE;
  dev_add(fb_node);
}