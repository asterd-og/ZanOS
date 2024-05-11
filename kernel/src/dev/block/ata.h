#pragma once

#include <types.h>

#define ATA_PRIMARY 0x1F0
#define ATA_SECONDARY 0x170

#define ATA_PRIMARY_CTRL 0x3F6
#define ATA_SECONDARY_CTRL 0x376

#define ATA_MASTER 0xA0
#define ATA_SLAVE 0xB0

#define ATA_WAIT 0x00
#define ATA_IDENTIFY 0xEC
#define ATA_READ 0x20
#define ATA_WRITE 0x30

extern char ata_name[40];

enum {
  ATA_OKAY,
  ATA_DISK_NOT_IDENTIFIED,
  ATA_DISK_ERR,
  ATA_DISK_NOT_READY
};

u8 ata_init();

u8 ata_read(u32 lba, u8* buffer, u32 sector_count);
u8 ata_write(u32 lba, u8* buffer, u32 sector_count);