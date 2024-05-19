#pragma once

#include <types.h>

#define PCI_BAR0 0x10

#define PCI_MAX_BUS 256
#define PCI_MAX_SLOT 32
#define PCI_MAX_FUNC 8

typedef struct {
  u8 device;
  u8 bus;
  u8 function;
  u8 _class;
  u8 subclass;
  u16 device_id;
  u16 vendor_id;
  u32 bars[6];
} pci_device;

void pci_init();

u16 pci_read_word(u8 bus, u8 slot, u8 func, u8 offset);
u32 pci_read_dword(u8 bus, u8 slot, u8 func, u8 offset);