#include <fs/ext2.h>
#include <lib/libc.h>
#include <lib/lock.h>
#include <mm/kmalloc.h>
#include <dev/dev.h>

atomic_lock* ext2_lock;

i32 ext2_read(vfs_node* vnode, u8* buffer, u32 count) {
  if (!(vnode->type == VFS_FILE)) {
    return -1;
  }
  lock(&ext2_lock);
  u32 ino_no = vnode->ino;
  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, ino_no, ino);
  ext2_read_inode_blocks(root_fs, ino, buffer);
  kfree(ino);
  unlock(&ext2_lock);
  return 0;
}

vfs_dirent* ext2_readdir(struct vfs_node* vnode, u32 index) {
  lock(&ext2_lock);
  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, vnode->ino, ino);
  u8* buf = (u8*)kmalloc((ino->sector_count / 2) * root_fs->block_size);
  u8* _buf = buf;
  ext2_read_inode_blocks(root_fs, ino, buf);

  ext2_dirent* dir = (ext2_dirent*)buf;
  u32 i = 0;

  while (dir->inode != 0) {
    dir = (ext2_dirent*)buf;
    buf += dir->total_size;
    if (i == index) {
      kfree(ino);
      if (dir->inode == 0) {
        unlock(&ext2_lock);
        kfree(_buf);
        return NULL;
      }
      vfs_dirent* dirent = (vfs_dirent*)kmalloc(sizeof(vfs_dirent));
      dirent->ino = dir->inode;
      dirent->name = (char*)kmalloc(dir->name_len);
      memcpy(dirent->name, dir->name, dir->name_len);
      kfree(_buf);
      unlock(&ext2_lock);
      return dirent;
    }
    i++;
  }
  kfree(_buf);
  kfree(ino);
  unlock(&ext2_lock);
  return NULL;
}

vfs_node* ext2_finddir(struct vfs_node* vnode, char* path) {
  lock(&ext2_lock);
  if (vnode == vfs_root && !strcmp(path, "dev")) {
    unlock(&ext2_lock);
    return vfs_dev;
  }

  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, vnode->ino, ino);
  u32 ino_no = ext2_get_inode(root_fs, ino, path);
  if (ino_no == 0) {
    kfree(ino);
    unlock(&ext2_lock);
    return NULL;
  }
  ext2_inode* dir_inode = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, ino_no, dir_inode);

  vfs_node* node = (vfs_node*)kmalloc(sizeof(vfs_node));
  u32 path_len = strlen(path);
  node->name = (char*)kmalloc(path_len);
  memcpy(node->name, path, path_len);
  node->perms |= VFS_DESTROY;
  if (dir_inode->type_perms & EXT_FILE)
    node->type = VFS_FILE;
  else if (dir_inode->type_perms & EXT_DIRECTORY)
    node->type = VFS_DIRECTORY;
  node->size = ALIGN_UP(dir_inode->size, root_fs->block_size); // To make sure it can read blocks
  // Without interfering in other allocations.
  node->ino = ino_no;
  node->read = ext2_read;
  node->readdir = ext2_readdir;
  node->finddir = ext2_finddir;
  
  kfree(ino);
  kfree(dir_inode);

  unlock(&ext2_lock);
  return node;
}