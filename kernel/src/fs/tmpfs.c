#include <fs/tmpfs.h>
#include <fs/ext2.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>
#include <mm/malloc.h>
#include <dev/char/serial.h>

ext2_fs* root_tmpfs = NULL;
u8* tmpfs_start = NULL;
vfs_node* vfs_tmpfs = NULL;

void ext2_tmpfs_read_block(ext2_fs* fs, u32 block, void* buf, u32 count) {
  memcpy(buf, tmpfs_start + (block * fs->block_size), count);
}

i32 tmpfs_read(vfs_node* vnode, u8* buffer, u32 count) {
  if (!(vnode->type == VFS_FILE)) {
    return -1;
  }
  u32 ino_no = vnode->ino;
  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_tmpfs, ino_no, ino);
  ext2_read_inode_blocks(root_tmpfs, ino, buffer, count);
  kfree(ino);
  return 0;
}

vfs_dirent* tmpfs_readdir(struct vfs_node* vnode, u32 index) {
  ext2_inode* ino = (ext2_inode*)kmalloc(root_tmpfs->inode_size);
  ext2_read_inode(root_tmpfs, vnode->ino, ino);
  u8* buf = (u8*)kmalloc((ino->sector_count / 2) * root_tmpfs->block_size);
  u8* _buf = buf;
  ext2_read_inode_blocks(root_tmpfs, ino, buf, ino->size);

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
      vfs_dirent* dirent = (vfs_dirent*)malloc(sizeof(vfs_dirent));
      dirent->ino = dir->inode;
      dirent->name = (char*)malloc(dir->name_len);
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

vfs_node* tmpfs_finddir(struct vfs_node* vnode, char* path) {
  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_tmpfs, vnode->ino, ino);
  u32 ino_no = ext2_get_inode(root_tmpfs, ino, path);
  if (ino_no == 0) {
    kfree(ino);
    return NULL;
  }
  ext2_inode* dir_inode = (ext2_inode*)kmalloc(sizeof(ext2_inode));
  ext2_read_inode(root_tmpfs, ino_no, dir_inode);

  vfs_node* node = (vfs_node*)kmalloc(sizeof(vfs_node));
  u32 path_len = strlen(path);
  node->parent = vnode;
  node->open = true;
  node->name = (char*)kmalloc(path_len);
  memcpy(node->name, path, path_len);
  node->perms |= VFS_DESTROY;
  if (dir_inode->type_perms & EXT_FILE)
    node->type = VFS_FILE;
  else if (dir_inode->type_perms & EXT_DIRECTORY)
    node->type = VFS_DIRECTORY;
  node->size = dir_inode->size; // To make sure it can read blocks
  // Without interfering in other allocations.
  node->ino = ino_no;
  node->read = tmpfs_read;
  node->readdir = tmpfs_readdir;
  node->finddir = tmpfs_finddir;
  
  kfree(ino);
  kfree(dir_inode);

  return node;
}

u8 tmpfs_init(u8* tmpfs_addr) {
  tmpfs_start = tmpfs_addr;
  ext2_fs* fs = (ext2_fs*)kmalloc(sizeof(ext2_fs));
  ext2_sb* sb = (ext2_sb*)kmalloc(sizeof(ext2_sb));
  memcpy(sb, tmpfs_start + (1024), 1024);
  if (sb->signature != 0xef53)
    return 1;
  
  fs->read_block = ext2_tmpfs_read_block;
  
  fs->sb = sb;
  fs->block_size = (1024 << sb->log2_block);
  fs->bgd_count = sb->blocks_count / sb->blocks_per_group;
  if (!fs->bgd_count) fs->bgd_count = 1;
  fs->bgd_block = sb->block_num + 1; // First block after sb
  fs->bgd_table = (ext2_bgd*)kmalloc(sizeof(ext2_bgd) * fs->bgd_count);
  ext2_read_block(fs, fs->bgd_block, fs->bgd_table, fs->block_size);
  fs->inode_size = (sb->major_ver == 1 ? sb->inode_size : 128);

  u32 inode = 2; // root dir
  ext2_inode* root_in = (ext2_inode*)kmalloc(fs->inode_size);
  ext2_read_inode(fs, inode, root_in);

  fs->root_ino = root_in;

  root_tmpfs = fs;

  // Setup vfs node

  vfs_tmpfs = (vfs_node*)kmalloc(sizeof(vfs_node));
  vfs_tmpfs->parent = vfs_root;
  vfs_tmpfs->open = true;
  vfs_tmpfs->name = (char*)kmalloc(6);
  memcpy(vfs_tmpfs->name, "tmpfs", 6);
  vfs_tmpfs->ino = 2;
  vfs_tmpfs->write = 0;
  vfs_tmpfs->read = tmpfs_read;
  vfs_tmpfs->readdir = tmpfs_readdir;
  vfs_tmpfs->finddir = tmpfs_finddir;
  vfs_tmpfs->size = 1024;
  vfs_tmpfs->type = VFS_DIRECTORY;

  return 0;
}