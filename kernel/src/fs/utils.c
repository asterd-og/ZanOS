#include <fs/ext2.h>
#include <lib/printf.h>

void ext2_list_dir(ext2_fs* fs, ext2_inode* in) {
  ext2_dirent* dir = (ext2_dirent*)kmalloc((in->sector_count / 2) * sizeof(ext2_dirent));
  // we divide by 2 because sector count is each 512 bytes, we read 1024 bytes per block
  u8* buf = (u8*)kmalloc((in->sector_count / 2) * fs->block_size);

  for (int i = 0; i < 12; i++) {
    u32 block = in->direct_block_ptr[i];
    if (block == 0) break;
    ext2_read_block(fs, block, buf + (i * fs->block_size));
  }

  dir = (ext2_dirent*)buf;
  while (dir->inode != 0) {
    printf("%s %d\n", dir->name, dir->type);
    buf += dir->total_size;
    dir = (ext2_dirent*)buf;
  }
}