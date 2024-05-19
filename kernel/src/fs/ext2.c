#include <fs/ext2.h>
#include <dev/block/ata.h>
#include <lib/printf.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>

// I'm basing myself off https://www.nongnu.org/ext2-doc/ext2.pdf (awesome doc)

ext2_fs* root_fs;

int ext2_get_cache(ext2_fs* fs, u32 block) {
  if (fs->block_cache_idx == 0) return -1;
  for (int i = 0; i < EXT_MAX_CACHE; i++) {
    if (fs->block_cache[i].block == 0)
      break;
    if (fs->block_cache[i].block == block)
      return i;
  }
  return -1;
}

void ext2_read_block(ext2_fs* fs, u32 block, void* buf) {
  int cache_num = ext2_get_cache(fs, block);
  if (cache_num == -1) {
    ata_read(block * (fs->block_size / 512), buf, (fs->block_size / 512));

    if (fs->block_cache_idx < 0x1024) {
      // TODO: Remember to update this whenever write is implemented
      fs->block_cache[fs->block_cache_idx].block = block;
      fs->block_cache[fs->block_cache_idx].data = (u8*)kmalloc(fs->block_size);
      memcpy(fs->block_cache[fs->block_cache_idx].data, buf, fs->block_size);
      fs->block_cache_idx++;
    }
    return;
  }
  memcpy(buf, fs->block_cache[cache_num].data, fs->block_size);
}

void ext2_read_inode(ext2_fs* fs, u32 inode, ext2_inode* in) {
  u32 bg = (inode - 1) / fs->sb->inodes_per_group;
  u32 idx = (inode - 1) % fs->sb->inodes_per_group;
  u32 bg_idx = (idx * fs->inode_size) / fs->block_size;

  u8* buf = (u8*)kmalloc(fs->block_size);
  ext2_read_block(fs, fs->bgd_table[bg].inode_table_block + bg_idx, buf);
  // now we have a "list" of inodes, we need to index our inode
  memcpy(in, (buf + (idx % (fs->block_size / fs->inode_size)) * fs->inode_size), fs->inode_size);
  kfree(buf);
}

u32 ext2_get_inode(ext2_fs* fs, ext2_inode* in, char* name) {
  // we divide by 2 because sector count is each 512 bytes, we read 1024 bytes per block
  u8* buf = (u8*)kmalloc((in->sector_count / 2) * fs->block_size);
  u8* _buf = buf;

  for (int i = 0; i < 12; i++) {
    u32 block = in->direct_block_ptr[i];
    if (block == 0) break;
    ext2_read_block(fs, block, buf + (i * fs->block_size));
  }

  int plen = strlen(name);

  ext2_dirent* dir = (ext2_dirent*)buf;
  while (dir->inode != 0) {
    dir = (ext2_dirent*)buf;
    buf += dir->total_size;
    if (!memcmp(dir->name, name, plen)) {
      kfree(_buf);
      return dir->inode;
    }
  }
  kfree(_buf);
  return 0;
}

void ext2_read_singly_blocks(ext2_fs* fs, u32 singly_block_id, u8* buf) {
  u32* blocks = (u32*)kmalloc(fs->block_size);
  u32 block_count = fs->block_size / 4; // on 1KB Blocks, 13 - 268 (or 256 blocks)
  ext2_read_block(fs, singly_block_id, blocks);
  for (int i = 0; i < block_count; i++) {
    if (blocks[i] == 0) break;
    ext2_read_block(fs, blocks[i], buf + (i * fs->block_size));
  }
  kfree(blocks);
}

void ext2_read_inode_blocks(ext2_fs* fs, ext2_inode* in, u8* buf) {
  // TODO: Read singly, doubly and triply linked blocks
  for (int i = 0; i < 12; i++) {
    u32 block = in->direct_block_ptr[i];
    if (block == 0) break;
    ext2_read_block(fs, block, buf + (i * fs->block_size));
  }
  if (in->singly_block_ptr != 0) {
    ext2_read_singly_blocks(fs, in->singly_block_ptr, buf + (12 * fs->block_size));
  }
}

u8 ext2_init() {
  ext2_fs* fs = (ext2_fs*)kmalloc(sizeof(ext2_fs));
  ext2_sb* sb = (ext2_sb*)kmalloc(sizeof(ext2_sb));
  ata_read(2, (u8*)sb, 2);
  if (sb->signature != 0xef53)
    return 1;
  
  fs->sb = sb;
  fs->block_size = (1024 << sb->log2_block);
  fs->block_cache = (ext2_cache*)kmalloc(sizeof(ext2_cache) * EXT_MAX_CACHE);
  fs->block_cache_idx = 0;
  fs->bgd_count = sb->blocks_count / sb->blocks_per_group;
  if (!fs->bgd_count) fs->bgd_count = 1;
  fs->bgd_block = sb->block_num + 1; // First block after sb
  fs->bgd_table = (ext2_bgd*)kmalloc(sizeof(ext2_bgd) * fs->bgd_count);
  ext2_read_block(fs, fs->bgd_block, fs->bgd_table);
  fs->inode_size = (sb->major_ver == 1 ? sb->inode_size : fs->inode_size);

  u32 inode = 2; // root dir
  ext2_inode* root_in = (ext2_inode*)kmalloc(fs->inode_size);
  ext2_read_inode(fs, inode, root_in);

  fs->root_ino = root_in;

  root_fs = fs;

  return 0;
}