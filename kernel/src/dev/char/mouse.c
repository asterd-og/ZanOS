#include <dev/char/mouse.h>
#include <dev/video/fb.h>
#include <dev/char/serial.h>
#include <dev/dev.h>
#include <dev/lapic.h>
#include <lib/lock.h>
#include <lib/libc.h>
#include <sys/ports.h>
#include <sys/idt.h>
#include <fs/vfs.h>
#include <mm/kmalloc.h>

u8 mouse_state = 0;
u8 mouse_flags = 0;
i32 mouse_bytes[2] = {0, 0};

u32 mouse_x = 0;
u32 mouse_y = 0;

bool mouse_left_pressed = false;
bool mouse_right_pressed = false;

bool mouse_moved = false;

static inline void mouse_wait_write() {
  int timeout = 100000;
  while (timeout--) {
    if ((inb(0x64) & 2) == 0) {
      return;
    }
  }
}

static inline void mouse_wait_read() {
  int timeout = 100000;
  while (timeout--) {
    if ((inb(0x64) & 1) == 1) {
      return;
    }
  }
}

void mouse_write(u8 value) {
  mouse_wait_write();
  outb(0x64, 0xd4);
  mouse_wait_write();
  outb(0x60, value);
}

u8 mouse_read() {
  mouse_wait_read();
  return inb(0x60);
}

atomic_lock mlock;

i32 mouse_wrap_x = 20;
i32 mouse_wrap_y = 20;

void mouse_handler(registers* regs) {
  (void)regs;
  static bool discard_packet = false;
  switch (mouse_state) {
    // Packet state
    case 0:
      mouse_flags = mouse_read();
      mouse_state++;
      if (mouse_flags & (1 << 6) || mouse_flags & (1 << 7))
        discard_packet = true;
      if (!(mouse_flags & (1 << 3)))
        discard_packet = true;
      break;
    case 1:
      mouse_bytes[0] = mouse_read();
      mouse_state++;
      break;
    case 2:
      mouse_bytes[1] = mouse_read();
      mouse_state = 0;

      if (!discard_packet) {
        if (mouse_flags & (1 << 4))
            mouse_bytes[0] = (i8)(u8)mouse_bytes[0];
        if (mouse_flags & (1 << 5))
            mouse_bytes[1] = (i8)(u8)mouse_bytes[1];

        mouse_x = mouse_x + (mouse_bytes[0]);
        mouse_y = mouse_y - (mouse_bytes[1]);

        if (mouse_x <= 1) mouse_x = 1;
        if (mouse_y <= 1) mouse_y = 1;
        if (mouse_x >= fb->width - 1) mouse_x = fb->width - 1;
        if (mouse_y >= fb->height - 1) mouse_y = fb->height - 1;

        mouse_moved = true;
        mouse_left_pressed = (bool)(mouse_flags & 0b00000001);
        mouse_right_pressed = (bool)((mouse_flags & 0b00000010) >> 1);
      } else
        discard_packet = false;
      break;
  }
  lapic_eoi();
}

i32 mouse_vread(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode; (void)count;
  mouse_event* ev = (mouse_event*)buffer;
  ev->x = mouse_x;
  ev->y = mouse_y;
  ev->left_button = mouse_left_pressed;
  ev->right_button = mouse_right_pressed;
  return sizeof(mouse_event);
}

void mouse_init() {
  mouse_wait_write();
  outb(0x64, 0xA8); // Enable second ps/2 port

  mouse_write(0xf6); // Set defaults
  mouse_read();

  mouse_write(0xf4); // Enable data reporting
  mouse_read();

  vfs_node* mouse_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  mouse_node->name = (char*)kmalloc(5);
  memcpy(mouse_node->name, "mouse", 5);
  mouse_node->ino = 0;
  mouse_node->write = 0;
  mouse_node->read = mouse_vread;
  mouse_node->readdir = 0;
  mouse_node->finddir = 0;
  mouse_node->size = 1;
  mouse_node->type = VFS_DEVICE;
  dev_add(mouse_node);

  irq_register(12, mouse_handler);
}