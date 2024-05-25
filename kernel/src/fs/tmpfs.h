#pragma once

#include <types.h>
#include <fs/vfs.h>

extern vfs_node* vfs_tmpfs;

u8 tmpfs_init(u8* tmpfs_addr);