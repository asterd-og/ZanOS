#include <dev/char/serial.h>
#include <lib/lock.h>

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