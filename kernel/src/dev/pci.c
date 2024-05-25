#include <dev/pci.h>
#include <acpi/acpi.h>
#include <sys/ports.h>

pci_device pci_list[128];
i8 pci_list_idx = 0;

u16 pci_read_word(u8 bus, u8 slot, u8 func, u8 offset) {
  u32 addr;
  u32 lbus = (u32)bus;
  u32 lslot = (u32)slot;
  u32 lfunc = (u32)func;
  
  addr = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) |
         ((u32)0x80000000));
  
  outl(0xCF8, addr);

  return (u16)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

void pci_write_dword(u8 bus, u8 slot, u8 func, u8 offset, u32 value) {
  u32 addr;
  u32 lbus = (u32)bus;
  u32 lslot = (u32)slot;
  u32 lfunc = (u32)func;
  
  addr = (u32)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) |
         ((u32)0x80000000));
  
  outl(0xCF8, addr);
  outl(0xCFC, value);
}

u32 pci_read_dword(u8 bus, u8 slot, u8 func, u8 offset) {
  return ((u32)pci_read_word(bus, slot, func, offset + 2) << 16) | pci_read_word(bus, slot, func, offset);
}

void pci_add_dev(u8 bus, u8 func, u8 class, u8 subclass, u16 device_id, u16 vendor_id, u32* bars) {
  pci_list[pci_list_idx].bus = bus;
  pci_list[pci_list_idx].function = func;
  pci_list[pci_list_idx].class = class;
  pci_list[pci_list_idx].subclass = subclass;
  pci_list[pci_list_idx].device_id = device_id;
  pci_list[pci_list_idx].vendor_id = vendor_id;
  memcpy(pci_list[pci_list_idx].bars, bars, sizeof(u32) * 6);
  pci_list_idx++;
}

i8 pci_find_device(u8 class, u8 subclass) {
  for (i8 i = 0; i < pci_list_idx; i++) {
    if (pci_list[i].class == class && pci_list[i].subclass == subclass)
      return i;
  }
  return -1;
}

void pci_init() {
  u16 vendor, device;
  u16 class_subclass;
  u8 class, subclass;
  u32 bars[6];

  acpi_mcfg* mcfg = acpi_find_table("MCFG");

  mcfg_entry* entry = &mcfg->table[0];
  for (u64 bus = entry->first_bus; bus < entry->last_bus; bus++)
    for (u8 slot = 0; slot < PCI_MAX_SLOT; slot++)
      for (u8 func = 0; func < PCI_MAX_FUNC; func++) {
        vendor = pci_read_word(bus, slot, func, 0);
        if (vendor == 0xFFFF) continue;
        device = pci_read_word(bus, slot, func, 2);
        class_subclass = pci_read_word(bus, slot, func, 0x8 + 2);
        class = (u8)class_subclass;
        subclass = (u8)((class_subclass & 0xFF00) >> 8);
        for (int i = 0; i < 6; i++) {
          bars[i] = pci_read_dword(bus, slot, func, PCI_BAR0 + (sizeof(u32) * i));
        }

        pci_add_dev(bus, func, class, subclass, device, vendor, bars);
      }
}