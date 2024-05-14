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

u32 vfs_read(struct vfs_node* vnode, u32 offset, u32 count, u8* buffer) {
  if (vnode->read && vnode->type == VFS_FILE)
    return vnode->read(vnode, offset, count, buffer);
  return 1;
}

vfs_dirent* vfs_readdir(struct vfs_node* vnode, u32 index) {
  if (vnode->readdir && vnode->type == VFS_DIRECTORY)
    return vnode->readdir(vnode, index);
  return NULL;
}

vfs_node* vfs_finddir(struct vfs_node* vnode, char* path) {
  if (vnode->readdir && vnode->type == VFS_DIRECTORY)
    return vnode->finddir(vnode, path);
  return NULL;
}