#pragma once

#include <types.h>

#define PCI_BAR0 0x10

#define PCI_MAX_BUS 256
#define PCI_MAX_SLOT 32
#define PCI_MAX_FUNC 8

typedef struct {
  u64 addr;
  u16 seg;
  u8 first_bus;
  u8 last_bus;
  u32 resv;
} __attribute__((packed)) mcfg_entry;

typedef struct {
  char sign[4];
  u32 len;
  u8 revision;
  u8 checksum;
  char oem_id[6];
  char oem_table_id[8];
  u32 oem_revision;
  u32 creator_id;
  u32 creator_revision;
  u64 resv;

  mcfg_entry table[];
}  __attribute__((packed)) acpi_mcfg;

typedef struct {
  u8 bus;
  u8 function;
  u8 class;
  u8 subclass;
  u16 device_id;
  u16 vendor_id;
  u32 bars[6];
} pci_device;

extern pci_device pci_list[128];

void pci_init();

u16 pci_read_word(u8 bus, u8 slot, u8 func, u8 offset);
u32 pci_read_dword(u8 bus, u8 slot, u8 func, u8 offset);

void pci_set_master_bus(pci_device dev);

i8 pci_find_device(u8 class, u8 subclass);