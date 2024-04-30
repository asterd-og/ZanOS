#pragma once

#include <types.h>

#include <acpi/acpi.h>
#include <acpi/madt.h>

#define IOAPIC_REGSEL 0x0
#define IOAPIC_IOWIN  0x10

#define IOAPIC_ID     0x0
#define IOAPIC_VER    0x01
#define IOAPIC_ARB    0x02
#define IOAPIC_REDTBL 0x10

u64 ioapic_init();

void ioapic_write(madt_ioapic* ioapic, u8 reg, u32 val);
u32 ioapic_read(madt_ioapic* ioapic, u8 reg);

void ioapic_redirect_irq(u32 lapic_id, u8 vec, u8 irq, bool mask);
u32 ioapic_get_redirect_irq(u8 irq);