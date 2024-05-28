[bits 64]
[global syscall_entry]

syscall_entry:
  int 0x80
  o64 sysret