[global jump_usermode]
[bits 64]

[extern test_func]

jump_usermode:
  push qword 0x3B  ; SS Data segment (GDT Entry offset for user mode data + RPL 3)
  push qword rdi   ; Stack
  push qword 0x202 ; Rflags
  push qword 0x43  ; CS Code segment (GDT Entry offset for user mode code + RPL 3)
  push qword rsi   ; Entry point
  iretq