#include <fs/ext2.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>
#include <dev/dev.h>

i32 ext2_read(struct vfs_node* vnode, u8* buffer, u32 count) {
  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, vnode->ino, ino);
  if (!(ino->type_perms & EXT_FILE))
    return 1;
  ext2_read_inode_blocks(root_fs, ino, buffer);
  kfree(ino);
  return 0;
}

vfs_dirent* ext2_readdir(struct vfs_node* vnode, u32 index) {
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
        kfree(_buf);
        return NULL;
      }
      vfs_dirent* dirent = (vfs_dirent*)kmalloc(sizeof(vfs_dirent));
      dirent->ino = dir->inode;
      dirent->name = (char*)kmalloc(dir->name_len);
      dprintf("%d '%s'\n", dir->name_len, dir->name);
      memcpy(dirent->name, dir->name, dir->name_len);
      kfree(_buf);

      return dirent;
    }
    i++;
  }
  kfree(_buf);
  kfree(ino);
  return NULL;
}

vfs_node* ext2_finddir(struct vfs_node* vnode, char* path) {
  if (vnode == vfs_root && !strcmp(path, "dev"))
    return vfs_dev;

  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, vnode->ino, ino);
  u32 ino_no = ext2_get_inode(root_fs, ino, path);
  ext2_inode* dir_inode = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_fs, ino_no, dir_inode);

  vfs_node* node = (vfs_node*)kmalloc(sizeof(vfs_node));
  u32 path_len = strlen(path);
  node->name = (char*)kmalloc(path_len);
  memcpy(node->name, path, path_len);
  node->perms = 0; // TODO: Implement perms
  if (dir_inode->type_perms & EXT_FILE)
    node->type = VFS_FILE;
  else if (dir_inode->type_perms & EXT_DIRECTORY)
    node->type = VFS_DIRECTORY;
  node->size = dir_inode->size;
  node->ino = ino_no;
  node->read = ext2_read;
  node->readdir = ext2_readdir;
  node->finddir = ext2_finddir;
  
  kfree(ino);
  kfree(dir_inode);

  return node;
}