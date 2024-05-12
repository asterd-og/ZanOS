#include <fs/ext2.h>
#include <lib/libc.h>
#include <mm/kmalloc.h>

int ext2_read(char* path, u8* buffer, u32 offset, u32 count) {
  if (path[0] != '/')
    return 1;

  bool subdir = false;

  for (int i = 1; path[i] != 0; i++)
    if (path[i] == '/') {
      subdir = true;
    }

  if (!subdir) {
    ext2_read_file(root_fs, root_fs->root_ino, path + 1, buffer);
    return 0;
  }

  char* _path = (char*)kmalloc(strlen(path));
  memcpy(_path, path, strlen(path));

  ext2_inode* ino = (ext2_inode*)kmalloc(sizeof(ext2_inode*));
  u32 ino_no = 0;

  char* token = strtok(_path, "/");

  while (token) {
    if (ino_no == 0) {
      ino_no = ext2_get_inode(root_fs, root_fs->root_ino, token);
    } else {
      ino_no = ext2_get_inode(root_fs, ino, token);
    }
    ext2_read_inode(root_fs, ino_no, ino);
    if (ino->type_perms & EXT_FILE)
      break;
    token = strtok(NULL, "/");
  }

  ext2_read_inode_blocks(root_fs, ino, buffer);
  return 0;
}