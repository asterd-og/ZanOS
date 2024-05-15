#include <fs/vfs.h>
#include <fs/ext2.h>
#include <mm/kmalloc.h>

vfs_node* vfs_root;

void vfs_init() {
  vfs_root = (vfs_node*)kmalloc(sizeof(vfs_node));
  vfs_root->name = kmalloc(2);
  vfs_root->name[0] = '/';
  vfs_root->name[1] = '\0';
  vfs_root->ino = 2;
  vfs_root->read = NULL;
  vfs_root->readdir = ext2_readdir;
  vfs_root->finddir = ext2_finddir;
  vfs_root->size = root_fs->root_ino->size;
  vfs_root->type = VFS_DIRECTORY;
}

i32 vfs_write(vfs_node* vnode, u8* buffer, u32 count) {
  if (vnode->write)
    return vnode->write(vnode, buffer, count);
  return -1;
}

i32 vfs_read(vfs_node* vnode, u8* buffer, u32 count) {
  if (vnode->read)
    return vnode->read(vnode, buffer, count);
  return -1;
}

vfs_dirent* vfs_readdir(vfs_node* vnode, u32 index) {
  if (vnode->readdir && vnode->type == VFS_DIRECTORY)
    return vnode->readdir(vnode, index);
  return NULL;
}

vfs_node* vfs_finddir(vfs_node* vnode, char* path) {
  if (vnode->finddir && vnode->type == VFS_DIRECTORY)
    return vnode->finddir(vnode, path);
  return NULL;
}