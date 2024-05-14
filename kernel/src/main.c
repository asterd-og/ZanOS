// #define FLANTERM_SCALE_DEFAULT
#include <kernel.h>
#include <types.h>
#include <limine.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/smp.h>
#include <lib/printf.h>
#include <lib/libc.h>
#include <flanterm/flanterm.h>
#include <flanterm/backends/fb.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <mm/kmalloc.h>
#include <acpi/acpi.h>
#include <acpi/madt.h>
#include <dev/lapic.h>
#include <dev/ioapic.h>
#include <dev/pit.h>
#include <dev/block/ata.h>
#include <sched/sched.h>
#include <fs/ext2.h>
#include <fs/vfs.h>

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

void putchar_(char c) {
  char str[1] = {c};
  flanterm_write(ft_ctx, str, 1);
}

void hcf() {
  for (;;) __asm__ volatile ("hlt");
}

void task() {
  printf("Hello from task 1!\nGoing to sleep for 1 second (1000 ms)\n");
  sleep(1000);
  printf("Just woke up.\n");
  for (;;) {
  }
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

  u32 defaultbg = 0x1b1c1b;
  u32 defaultfg = 0xffffff;

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
    &defaultbg, &defaultfg,
    NULL, NULL,
    NULL, 0, 0, 1,
    0, 0,
    15
  );

  gdt_init();
  idt_init();
  pmm_init();
  vmm_init();
  kheap_init();
  if (!acpi_init()) {
    printf("acpi_init(): Couldn't find ACPI.\n");
    hcf();
  }
  madt_init();
  lapic_init();
  ioapic_init();
  pit_init();
  smp_init();
  lapic_calibrate_timer();
  sched_init();
  dprintf("_start(): Initialised scheduler.\n");
  ata_init();
  printf("\033[38;2;0;255;255mZanOS\033[0m Booted successfully with %ld cores.\n", smp_cpu_count);
  ext2_init();
  vfs_init();

  int i = 0;
  vfs_dirent* node = 0;

  while ((node = vfs_readdir(vfs_root, i)) != NULL) {
    printf("Found node: ");
    vfs_node* file = vfs_finddir(vfs_root, node->name);
    if (file->type == VFS_DIRECTORY)
      printf("\033[38;2;0;255;0m%s\033[0m\n", file->name);
    else {
      printf("%s\n", file->name);
      printf(" contents: ");
      char buf[1024];
      vfs_read(file, 0, 0, buf);
      printf("%s\n", buf);
    }
    i++;
  }

  sched_new_task(task, 1);

  irq_register(0x32 - 32, sched_schedule);
  lapic_send_all_int(bsp_lapic_id, 0x32); // Jumpstart the scheduler
  
  while (true) {
    __asm__ volatile ("hlt");
  }
}