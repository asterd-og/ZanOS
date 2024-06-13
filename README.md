# ZanOS
- A x86_64 operating system made from scratch by astrido!

# Features
- Paging
- APIC/LAPIC/IOAPIC
- Multiprocessing multitasking
- Lapic timer
- ATA PIO Driver
- EXT2 Driver (read-only only, write WIP)
- VFS Driver (WIP)
- User mode
- ELF Loading

# How to build?
You'll need to install `make gcc nasm xorriso qemu-system-x86` packages, after you install those, you'll just need to run `make` to build and `make run` (or just that to build and run already).
You will need to also make a disk.img file, to do that just run `qemu-img create disk.img 20M` and `mkfs.ext2 disk.img -b1024`.

# Screenshots
![ext2_vfs](https://github.com/asterd-og/ZanOS/blob/main/images/ext2_vfs.png?raw=true)