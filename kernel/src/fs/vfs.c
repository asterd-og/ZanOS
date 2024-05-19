#include <fs/vfs.h>
#include <fs/ext2.h>
#include <mm/malloc.h>
#include <mm/kmalloc.h>
#include <lib/libc.h>
#include <lib/lock.h>

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
  if (!vnode) return -1;
  if (vnode->write)
    return vnode->write(vnode, buffer, count);
  return -1;
}

i32 vfs_read(vfs_node* vnode, u8* buffer, u32 count) {
  if (!vnode) return -1;
  if (vnode->read)
    return vnode->read(vnode, buffer, count);
  return -1;
}

vfs_dirent* vfs_readdir(vfs_node* vnode, u32 index) {
  if (!vnode) return -1;
  if (vnode->readdir && vnode->type == VFS_DIRECTORY)
    return vnode->readdir(vnode, index);
  return NULL;
}

vfs_node* vfs_finddir(vfs_node* vnode, char* path) {
  if (!vnode) return -1;
  if (vnode->finddir && vnode->type == VFS_DIRECTORY)
    return vnode->finddir(vnode, path);
  return NULL;
}

vfs_node* vfs_open(vfs_node* vnode, char* path) {
  bool root = (path[0] == '/');
  int plen = strlen(path);

  bool has_subdir = false;
  for (int i = (root ? 1 : 0); i < plen; i++)
    if (path[i] == '/') {
      has_subdir = true;
      break; // it has subdirs
    }

  if (!has_subdir)
    return vfs_finddir((root ? vfs_root : vnode), path + (root ? 1 : 0));

  char* _path = (char*)malloc(plen);
  memcpy(_path, (root ? path + 1 : path), plen);

  char* token = strtok(_path, "/");
  vfs_node* current = (root ? vfs_root : vnode);

  while (token) {
    current = vfs_finddir(current, token);
    if (current == NULL) {
      free(_path);
      return NULL;
    }

    token = strtok(NULL, "/");
  }

  return current;
}

void vfs_destroy(vfs_node* vnode) {
  if (!(vnode->perms & VFS_DESTROY))
    return;
  
  free(vnode->name);
  free(vnode);
}