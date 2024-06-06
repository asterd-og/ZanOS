#include <kernel.h>
#include <dev/video/fb.h>
#include <dev/dev.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>

vfs_node* fb_node;
fb_info* fb;

i32 fb_write(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode;
  memcpy(framebuffer->address, buffer, count);
  return count;
}

i32 fb_read(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode; (void)count;
  memcpy(buffer, fb, sizeof(fb_info));
  return sizeof(fb_info);
}

void fb_init() {
  fb = (fb_info*)kmalloc(sizeof(fb_info));
  fb->width = framebuffer->width;
  fb->height = framebuffer->height;
  fb->pitch = framebuffer->pitch;
  fb->bpp = framebuffer->bpp;

  fb_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  fb_node->name = (char*)kmalloc(4);
  memcpy(fb_node->name, "fb0", 4);
  fb_node->ino = 0;
  fb_node->write = fb_write;
  fb_node->read = fb_read;
  fb_node->readdir = 0;
  fb_node->finddir = 0;
  fb_node->size = 1;
  fb_node->type = VFS_DEVICE;
  dev_add(fb_node);
}