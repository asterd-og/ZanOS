#include <dev/block/ata.h>
#include <sys/ports.h>

u16 ata_base = 0;
u8 ata_type = 0;
char ata_name[40];

u8 ata_poll() {
  for (u8 i = 0; i < 4; i++)
    inb(ata_base + 7); // 400ns delay
  
  u8 status = 0;
  for (;;) {
    status = inb(ata_base + 7);
    if (!(status & 0x80)) break;
    if (status & 0x08) break;
    else if (status & 0x01) return ATA_DISK_ERR;
  }

  return ATA_OKAY;
}

u8 ata_identify(u16 ata, u8 type) {
  ata_base = ata;
  ata_type = type;

  // Select disk
  outb(ata + 6, type); // master or slave
  for (u16 i = 0x1F2; i != 0x1F5; i++)
    outb(i, 0);
  outb(ata + 7, ATA_IDENTIFY);
  
  u8 status = inb(ata + 7);
  if (status == 0) return ATA_DISK_NOT_IDENTIFIED;

  if (ata_poll() != ATA_OKAY)
    return ATA_DISK_ERR;

  u8 buf[512];
  ata_read(0, buf, 1);

  for (u8 i = 0; i < 40; i += 2) {
    ata_name[i] = buf[54 + i + 1];
    ata_name[i + 1] = buf[54 + i];
  }
  
  return ATA_OKAY;
}

u8 ata_read(u32 lba, u8* buffer, u32 sector_count) {
  outb(ata_base + 6, (ata_type == ATA_MASTER ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F)); // Set master/slave
  outb(ata_base + 1, ATA_WAIT); // Send wait
  outb(ata_base + 2, sector_count); // Sector count
  outb(ata_base + 3, (u8)lba); // Start sending LBA
  outb(ata_base + 4, (u8)(lba >> 8));
  outb(ata_base + 5, (u8)(lba >> 16)); // 24-bit LBA addressing
  outb(ata_base + 7, ATA_READ);

  u16 val = 0;
  u32 i = 0;

  if (ata_poll() != ATA_OKAY)
    return ATA_DISK_ERR;

  for (; i < sector_count * 512; i += 2) {
    val = inw(ata_base);
    buffer[i] = val & 0x00ff;
    if (i + 1 < sector_count * 512)
      buffer[i + 1] = (val >> 8) & 0x00ff;
  }

  return ATA_OKAY;
}

u8 ata_write(u32 lba, u8* buffer, u32 sector_count) {
  outb(ata_base + 6, (ata_type == ATA_MASTER ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
  outb(ata_base + 1, ATA_WAIT);
  outb(ata_base + 2, sector_count);
  outb(ata_base + 3, (u8)lba);
  outb(ata_base + 4, (u8)(lba >> 8));
  outb(ata_base + 5, (u8)(lba >> 16));
  outb(ata_base + 7, ATA_WRITE);

  u16 val = 0;
  u32 i = 0;

  if (ata_poll() != ATA_OKAY)
    return ATA_DISK_ERR;

  for (; i < sector_count * 512; i += 2) {
    val = buffer[i];
    if (i + 1 < sector_count * 512)
      val |= ((u16)buffer[i + 1] << 8);
    outw(ata_base, val);
  }
  
  return ATA_OKAY;
}

u8 ata_init() {
  u8 ata_status = ata_identify(ATA_PRIMARY, ATA_MASTER);
  return ata_status;
}