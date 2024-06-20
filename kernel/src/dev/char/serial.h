#pragma once

#include <types.h>
#include <lib/printf.h>
#include <sys/ports.h>
#include <fs/vfs.h>

extern vfs_node* serial_node;

void dprintf(const char* fmt, ...);
void serial_init();