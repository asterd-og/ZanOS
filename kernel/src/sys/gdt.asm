[global gdt_flush]
gdt_flush:
  lgdt [rdi]

  push 0x28 ; 64 bit code seg
  lea rax, [rel .reload_cs]
  push rax
  retfq
.reload_cs:
  mov ax, 0x30 ; 64 bit data seg
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov ax, 0x48
  ltr ax
  ret