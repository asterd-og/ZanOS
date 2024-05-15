#pragma once

#include <types.h>
#include <fs/vfs.h>

extern vfs_node* vfs_dev;

void dev_init();
void dev_add(vfs_node* dev);