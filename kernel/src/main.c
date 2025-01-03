#include <kernel.h>
#include <types.h>
#include <limine.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/smp.h>
#include <sys/cpu.h>
#include <sys/user.h>
#include <lib/printf.h>
#include <lib/libc.h>
#include <lib/elf.h>
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <mm/malloc.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>
#include <dev/lapic.h>
#include <dev/ioapic.h>
#include <dev/pit.h>
#include <dev/tty.h>
#include <dev/dev.h>
#include <dev/pci.h>
#include <dev/block/ata.h>
#include <dev/char/keyboard.h>
#include <dev/char/mouse.h>
#include <dev/video/fb.h>
#include <sched/sched.h>
#include <sched/signal.h>
#include <fs/ext2.h>
#include <fs/vfs.h>
#include <fs/fs.h>
#include <fs/tmpfs.h>

// See specification for further info.

LIMINE_BASE_REVISION(1)

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, in C, they should
// NOT be made "static".

struct limine_framebuffer_request framebuffer_request = {
  .id = LIMINE_FRAMEBUFFER_REQUEST,
  .revision = 0
};
struct limine_framebuffer *framebuffer = NULL;
struct flanterm_context *ft_ctx = NULL;

// HHDM

struct limine_hhdm_request hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};

u64 hhdm_offset = 0;

struct limine_module_request module_request = {
  .id = LIMINE_MODULE_REQUEST,
  .revision = 0
};

void* get_mod_addr(int pos) {
  return module_request.response->modules[pos]->address;
}

struct limine_stack_size_request stack_sz_request = {
  .id = LIMINE_STACK_SIZE_REQUEST,
  .revision = 0
};

void putchar_(char c) {
  static atomic_lock l;
  lock(&l);
  char str[1] = {c};
  flanterm_write(ft_ctx, str, 1);
  unlock(&l);
}

void hcf() {
  for (;;) __asm__ volatile ("hlt");
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    hcf();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL
      || framebuffer_request.response->framebuffer_count < 1) {
    hcf();
  }

  hhdm_offset = hhdm_request.response->offset;
  framebuffer = framebuffer_request.response->framebuffers[0];

  ft_ctx = flanterm_fb_init(
    NULL,
    NULL,
    framebuffer->address, framebuffer->width,
    framebuffer->height, framebuffer->pitch,
    framebuffer->red_mask_size, framebuffer->red_mask_shift,
    framebuffer->green_mask_size, framebuffer->green_mask_shift,
    framebuffer->blue_mask_size, framebuffer->blue_mask_shift,
    NULL,
    NULL, NULL,
    NULL, NULL,
    NULL, NULL,
    NULL, 0, 0, 1,
    1, 1,
    0
  );

  gdt_init();
  idt_init();
  pmm_init();
  vmm_init();
  
  void* stack = HIGHER_HALF(pmm_alloc(3) + (3 * PAGE_SIZE));
  tss_list[0].rsp[0] = (u64)stack;

  kheap_init();
  dprintf("KHeap initialised.\n");
  if (!acpi_init()) {
    printf("acpi_init(): Couldn't find ACPI.\n");
    hcf();
  }
  madt_init();
  lapic_init();
  ioapic_init();
  pit_init();
  irq_register(0x80 - 32, sched_schedule);
  sse_enable();
  if (fpu_init()) {
    printf("This CPU does NOT support FPU.\n");
    hcf();
  }

  smp_init();
  sched_init();
  user_init();
  lapic_calibrate_timer();
  dprintf("_start(): Initialised scheduler.\n");
  ata_init();
  printf("\033[38;2;0;255;255mZanOS\033[0m Booted successfully with %ld cores.\n", smp_cpu_count);
  ext2_init();
  vfs_init();
  tmpfs_init(get_mod_addr(0));
  dev_init();
  serial_init();
  keyboard_init();
  tty_init();
  fb_init();
  mouse_init();

  process* proc = sched_new_proc("Test", SCHED_USER, 1, false);
  proc_add_elf_thread(proc, "/tmpfs/bin/test");

  lapic_ipi(bsp_lapic_id, 0x80);

  while (true) {
    __asm__ volatile ("hlt");
  }
}
