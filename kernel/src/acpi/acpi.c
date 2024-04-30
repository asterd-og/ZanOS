#include <acpi/acpi.h>
#include <dev/char/serial.h>

struct limine_rsdp_request rsdp_request = {
  .id = LIMINE_RSDP_REQUEST,
  .revision = 0
};

bool acpi_use_xsdt = false;

void* acpi_root_sdt;

void* acpi_find_table(const char* name) {
  if (acpi_use_xsdt == false) {
    acpi_rsdt* rsdt = (acpi_rsdt*)acpi_root_sdt;
    u32 entries = (rsdt->sdt.len - sizeof(rsdt->sdt)) / 4;

    for (u32 i = 0; i < entries; i++) {
      acpi_sdt* sdt = (acpi_sdt*)HIGHER_HALF(*((u32*)rsdt->table + i));
      if (!memcmp(sdt->sign, name, 4))
        return (void*)sdt;
    }
    return NULL;
  }

  // Use XSDT
  acpi_xsdt* xsdt = (acpi_xsdt*)acpi_root_sdt;
  u32 entries = (xsdt->sdt.len - sizeof(xsdt->sdt)) / 8;

  for (u32 i = 0; i < entries; i++) {
    acpi_sdt* sdt = (acpi_sdt*)HIGHER_HALF(*((u64*)xsdt->table + i));
    if (!memcmp(sdt->sign, name, 4)) {
      return (void*)sdt;
    }
  }

  return NULL;
}

u64 acpi_init() {
  void* addr = (void*)rsdp_request.response->address;
  acpi_rsdp* rsdp = (acpi_rsdp*)addr;

  if (memcmp(rsdp->sign, "RSD PTR", 7))
    return 0;

  if (rsdp->revision != 0) {
    // Use XSDT
    acpi_use_xsdt = true;
    acpi_xsdp* xsdp = (acpi_xsdp*)addr;
    acpi_root_sdt = (acpi_xsdt*)HIGHER_HALF(xsdp->xsdt_addr);
    return xsdp->xsdt_addr;
  }
  
  acpi_root_sdt = (acpi_rsdt*)HIGHER_HALF((u64)rsdp->rsdt_addr);

  dprintf("acpi_init(): Found %s.\n", (acpi_use_xsdt ? "XSDT" : "RSDT"));

  return rsdp->rsdt_addr;
}