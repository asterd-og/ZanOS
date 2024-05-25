#include <dev/dev.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>

vfs_node* vfs_dev;

vfs_node* devices[256];
u32 dev_index = 0;

vfs_dirent* dev_readdir(vfs_node* vnode, u32 index) {
  (void)vnode;
  if (devices[index] == NULL)
    return NULL;
  
  vfs_dirent* dirent = (vfs_dirent*)kmalloc(sizeof(vfs_dirent));
  dirent->name = devices[index]->name;
  dirent->ino = devices[index]->ino;

  return dirent;
}

vfs_node* dev_finddir(vfs_node* vnode, char* path) {
  (void)vnode;
  for (u32 i = 0; i < dev_index; i++)
    if (!strcmp(devices[i]->name, path))
      return devices[i];
  return NULL;
}

void dev_add(vfs_node* vnode) {
  devices[dev_index] = vnode;
  dev_index++;
}

void dev_init() {
  vfs_dev = (vfs_node*)kmalloc(sizeof(vfs_node));
  vfs_dev->parent = vfs_root;
  vfs_dev->open = true;
  vfs_dev->name = (char*)kmalloc(4);
  memcpy(vfs_dev->name, "dev", 4);
  vfs_dev->ino = 0;
  vfs_dev->write = 0;
  vfs_dev->read = 0;
  vfs_dev->readdir = dev_readdir;
  vfs_dev->finddir = dev_finddir;
  vfs_dev->size = 1;
  vfs_dev->type = VFS_DIRECTORY;
}