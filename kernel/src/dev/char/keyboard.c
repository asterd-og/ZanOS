#include <dev/char/keyboard.h>
#include <dev/char/keyboard_map.h>
#include <dev/lapic.h>
#include <dev/dev.h>
#include <sys/idt.h>
#include <sys/ports.h>
#include <lib/lock.h>
#include <mm/kmalloc.h>

bool keyboard_pressed = false;

u32 keyboard_char = '\0';
bool keyboard_caps = false;
bool keyboard_shift = false;

atomic_lock kb_lock;

void keyboard_handle_key(u8 key) {
  lock(&kb_lock);

  switch (key) {
    case 0x2a:
      // Shift
      keyboard_shift = true;
      break;
    case 0xaa:
      keyboard_shift = false;
      break;
    case 0x3a:
      // Caps
      keyboard_caps = !keyboard_caps;
      break;
    default:
      // Letter
      if (!(key & 0x80)) {
        keyboard_pressed = true;
        if (keyboard_shift) keyboard_char = kb_map_keys_shift[key];
        else if (keyboard_caps) keyboard_char = kb_map_keys_caps[key];
        else keyboard_char = kb_map_keys[key];
      }
      break;
  }
  unlock(&kb_lock);
}

void keyboard_handler(registers* regs) {
  (void)regs;

  u8 status;
  u8 key;

  lapic_eoi();

  status = inb(0x64);
  if (status & 1) {
    key = inb(0x60);
    keyboard_handle_key(key);
  } else {
    keyboard_char = 0;
    keyboard_pressed = false;
  }
  lapic_eoi();
}

i32 keyboard_read(struct vfs_node* vnode, u8* buffer, u32 count) {
  (void)vnode; (void)count;
  char c = keyboard_char;
  keyboard_event ev;
  ev.type = 1; // key
  ev.value = 1; // pressed
  ev.code = c;
  keyboard_char = 0;
  memcpy(buffer, &ev, sizeof(keyboard_event));
  return sizeof(keyboard_event);
}

void keyboard_init() {
  vfs_node* kb_node = (vfs_node*)kmalloc(sizeof(vfs_node));
  kb_node->name = (char*)kmalloc(9);
  memcpy(kb_node->name, "keyboard", 9);
  kb_node->ino = 0;
  kb_node->write = 0;
  kb_node->read = keyboard_read;
  kb_node->readdir = 0;
  kb_node->finddir = 0;
  kb_node->size = 1;
  kb_node->type = VFS_DEVICE;
  dev_add(kb_node);

  irq_register(1, keyboard_handler);
  inb(0x60);
}