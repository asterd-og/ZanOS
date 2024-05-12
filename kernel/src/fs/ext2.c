#include <fs/ext2.h>
#include <dev/block/ata.h>
#include <lib/printf.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>

// I'm basing myself off https://www.nongnu.org/ext2-doc/ext2.pdf (awesome doc)

ext2_cache block_cache[EXT_MAX_CACHE];
u32 block_cache_idx = 0;

int ext2_get_cache(u32 block) {
  if (block_cache_idx == 0) return -1;
  for (int i = 0; i < EXT_MAX_CACHE; i++) {
    if (block_cache[i].block == 0)
      break;
    if (block_cache[i].block == block)
      return i;
  }
  return -1;
}

void ext2_read_block(ext2_fs* fs, u32 block, void* buf) {
  int cache_num = ext2_get_cache(block);
  if (cache_num == -1) {
    ata_read(block * (fs->block_size / 512), buf, (fs->block_size / 512));

    if (block_cache_idx < 0x1024) {
      // TODO: Remember to update this whenever write is implemented
      block_cache[block_cache_idx].block = block;
      memcpy(block_cache[block_cache_idx].data, buf, 1024);
      block_cache_idx++;
    }
    return;
  }
  memcpy(buf, block_cache[cache_num].data, 1024);
}

void ext2_read_inode(ext2_fs* fs, u32 inode, ext2_inode* in) {
  u32 bg = (inode - 1) / fs->sb->inodes_per_group;
  u32 idx = (inode - 1) % fs->sb->inodes_per_group;
  u32 bg_idx = (idx * fs->inode_size) / fs->block_size;

  u8 buf[fs->block_size];
  ext2_read_block(fs, fs->bgd_table[bg].inode_table_block + bg_idx, buf);
  // now we have a "list" of inodes, we need to index our inode
  memcpy(in, (buf + (idx % (fs->block_size / fs->inode_size)) * fs->inode_size), fs->inode_size);
}

u32 ext2_get_inode(ext2_fs* fs, ext2_inode* in, char* name) {
  ext2_dirent* dir = (ext2_dirent*)kmalloc((in->sector_count / 2) * sizeof(ext2_dirent));
  // we divide by 2 because sector count is each 512 bytes, we read 1024 bytes per block
  u8* buf = (u8*)kmalloc((in->sector_count / 2) * fs->block_size);

  ext2_read_block(fs, in->direct_block_ptr[0], buf);

  do {
    dir = (ext2_dirent*)buf;
    buf += dir->total_size;
    if (!strcmp(dir->name, name)) {
      return dir->inode;
    }
  } while (dir->inode != 0);
  return 0;
}

void ext2_read_file(ext2_fs* fs, ext2_inode* in, char* name, u8* buf) {
  u32 ino = ext2_get_inode(fs, in, name);
  ext2_inode* inode = (ext2_inode*)kmalloc(fs->inode_size);
  ext2_read_inode(fs, ino, inode);

  for (int i = 0; i < 12; i++) {
    u32 block = inode->direct_block_ptr[i];
    if (block == 0) break;
    ext2_read_block(fs, block, buf + (i * fs->block_size));
  }
  // TODO: Read singly linked, doubly linked and triply linked
}

u8 ext2_init() {
  ext2_fs* fs = (ext2_fs*)kmalloc(sizeof(ext2_fs));
  ext2_sb* sb = (ext2_sb*)kmalloc(sizeof(ext2_sb));
  ata_read(2, (u8*)sb, 2);
  if (sb->signature != 0xef53)
    return 1;
  
  fs->sb = sb;
  fs->block_size = (1024 << sb->log2_block);
  fs->bgd_count = sb->blocks_count / sb->blocks_per_group;
  if (!fs->bgd_count) fs->bgd_count = 1;
  fs->bgd_block = sb->block_num + 1; // First block after sb
  fs->bgd_table = (ext2_bgd*)kmalloc(sizeof(ext2_bgd) * fs->bgd_count);
  ext2_read_block(fs, fs->bgd_block, fs->bgd_table);
  fs->inode_size = (sb->major_ver == 1 ? sb->inode_size : fs->inode_size);

  u32 inode = 2; // root dir
  ext2_inode* root_in = (ext2_inode*)kmalloc(fs->inode_size);
  ext2_read_inode(fs, inode, root_in);

  return 0;
}