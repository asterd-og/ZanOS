#include <fs/ext2.h>
#include <dev/char/serial.h>
#include <dev/block/ata.h>
#include <lib/printf.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>

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

void ext2_ata_read_block(ext2_fs* fs, u32 block, void* buf, u32 count) {
  int cache_num = ext2_get_cache(fs, block);
  if (cache_num == -1) {
    u8* disk_buf = kmalloc(fs->block_size);
    ata_read(block * (fs->block_size / 512), disk_buf, (fs->block_size / 512));
    memcpy(buf, disk_buf, count);
    kfree(disk_buf);

    if (fs->block_cache_idx < 0x1024) {
      // TODO: Remember to update this whenever write is implemented
      fs->block_cache[fs->block_cache_idx].block = block;
      fs->block_cache[fs->block_cache_idx].data = (u8*)kmalloc(fs->block_size);
      memcpy(fs->block_cache[fs->block_cache_idx].data, buf, count);
      fs->block_cache_idx++;
    }
    return;
  }
  memcpy(buf, fs->block_cache[cache_num].data, count);
}

void ext2_read_block(ext2_fs* fs, u32 block, void* buf, u32 count) {
  fs->read_block(fs, block, buf, count);
}

void ext2_read_inode(ext2_fs* fs, u32 inode, ext2_inode* in) {
  u32 bg = (inode - 1) / fs->sb->inodes_per_group;
  u32 idx = (inode - 1) % fs->sb->inodes_per_group;
  u32 bg_idx = (idx * fs->inode_size) / fs->block_size;

  u8* buf = (u8*)kmalloc(fs->block_size);
  ext2_read_block(fs, fs->bgd_table[bg].inode_table_block + bg_idx, buf, fs->block_size);
  // now we have a "list" of inodes, we need to index our inode
  memcpy(in, (buf + (idx % (fs->block_size / fs->inode_size)) * fs->inode_size), fs->inode_size);
  kfree(buf);
}

u32 ext2_get_inode(ext2_fs* fs, ext2_inode* in, char* name) {
  // we divide by 2 because sector count is each 512 bytes, we read 1024 bytes per block
  u8* buf = (u8*)kmalloc((in->sector_count / 2) * fs->block_size);
  u8* _buf = buf;

  ext2_read_inode_blocks(fs, in, buf, in->size);

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

u32 ext2_read_singly_blocks(ext2_fs* fs, u32 singly_block_id, u8* buf, u32 count) {
  u32* blocks = (u32*)kmalloc(fs->block_size);
  u32 block_count = fs->block_size / 4; // on 1KB Blocks, 13 - 268 (or 256 blocks)
  // Think about it this way: it's 1024 (block size) divided by 4 bytes (32 bit values)
  // and each 4 bytes is a block num
  u32 count_block = DIV_ROUND_UP(count, fs->block_size);
  ext2_read_block(fs, singly_block_id, blocks, fs->block_size);
  u32 remaining = count;
  for (u32 i = 0; i < count_block; i++) {
    if (i == block_count) break;
    if (blocks[i] == 0) break;
    ext2_read_block(fs, blocks[i], buf + (i * fs->block_size), (remaining > fs->block_size ? fs->block_size : remaining));
    remaining -= fs->block_size;
  }
  kfree(blocks);
  return remaining;
}

u32 ext2_read_doubly_blocks(ext2_fs* fs, u32 doubly_block_id, u8* buf, u32 count) {
  // A block containing an array of indirect block IDs
  // with each of those indirect blocks containing an array of blocks containing the data
  u32* blocks = (u32*)kmalloc(fs->block_size);
  u32 block_count = fs->block_size / 4; // on 1KB Blocks, 268 - 65804 (or 65536 blocks)

  u32 count_block = DIV_ROUND_UP(count, fs->block_size);
  ext2_read_block(fs, doubly_block_id, blocks, fs->block_size);
  u32 remaining = count;
  u32 rem_limit = fs->block_size * fs->block_size / 4;
  // rem_limit is the total of bytes in a singly linked list, divide that by the block size
  // and you get the amount of blocks in a singly linked list.

  for (u32 i = 0; i < count_block; i++) {
    if (i == block_count) break;
    if (blocks[i] == 0) break;
    ext2_read_singly_blocks(fs, blocks[i], buf + (i * rem_limit), (remaining > rem_limit ? rem_limit : remaining));
    remaining -= rem_limit;
  }
  kfree(blocks);
  return remaining;
}

void ext2_read_inode_blocks(ext2_fs* fs, ext2_inode* in, u8* buf, u32 count) {
  // TODO: Read singly, doubly and triply linked blocks
  u32 remaining = count;
  u32 blocks = DIV_ROUND_UP(count, fs->block_size);
  for (u32 i = 0; i < (blocks > 12 ? 12 : blocks); i++) {
    u32 block = in->direct_block_ptr[i];
    if (block == 0) break;
    ext2_read_block(fs, block, buf + (i * fs->block_size), (remaining > fs->block_size ? fs->block_size : remaining));
    remaining -= fs->block_size;
  }
  if (blocks > 12) {
    if (in->singly_block_ptr != 0) {
      ext2_read_singly_blocks(fs, in->singly_block_ptr, buf + (12 * fs->block_size), remaining);
    }
  }
  if (blocks > 265) {
    if (in->doubly_block_ptr != 0) {
      remaining -= fs->block_size * fs->block_size / 4;
      ext2_read_doubly_blocks(fs, in->singly_block_ptr, buf + (12 * fs->block_size) + (fs->block_size * fs->block_size / 4), remaining);
    }
  }
}

u8 ext2_init() {
  ext2_fs* fs = (ext2_fs*)kmalloc(sizeof(ext2_fs));
  ext2_sb* sb = (ext2_sb*)kmalloc(sizeof(ext2_sb));
  ata_read(2, (u8*)sb, 2);
  if (sb->signature != 0xef53)
    return 1;

  fs->read_block = ext2_ata_read_block;
  
  fs->sb = sb;
  fs->block_size = (1024 << sb->log2_block);
  fs->block_cache = (ext2_cache*)kmalloc(sizeof(ext2_cache) * EXT_MAX_CACHE);
  fs->block_cache_idx = 0;
  fs->bgd_count = sb->blocks_count / sb->blocks_per_group;
  if (!fs->bgd_count) fs->bgd_count = 1;
  fs->bgd_block = sb->block_num + 1; // First block after sb
  fs->bgd_table = (ext2_bgd*)kmalloc(sizeof(ext2_bgd) * fs->bgd_count);
  ext2_read_block(fs, fs->bgd_block, fs->bgd_table, fs->block_size);
  fs->inode_size = (sb->major_ver == 1 ? sb->inode_size : fs->inode_size);

  u32 inode = 2; // root dir
  ext2_inode* root_in = (ext2_inode*)kmalloc(fs->inode_size);
  ext2_read_inode(fs, inode, root_in);

  fs->root_ino = root_in;

  root_fs = fs;

  return 0;
}