#pragma once

#include <types.h>

typedef struct {
  u32 inodes_count;
  u32 blocks_count;
  u32 su_resv_blocks_count;
  u32 free_blocks_count;
  u32 free_inodes_count;
  u32 block_num;
  u32 log2_block;
  u32 log2_frag;
  u32 blocks_per_group;
  u32 frags_per_group;
  u32 inodes_per_group;
  u32 last_mount_time; // posix time
  u32 last_write_time; // posix time
  u16 mount_times_check; // times the volume has been mounted since it's last consistency check
  u16 mount_times_allowed;
  u16 signature;
  u16 state;
  u16 err_handle;
  u16 minor_ver;
  u32 last_consistency_check;
  u32 consistency_interval;
  u32 os_id;
  u32 major_ver;
  u16 resv_blocks_user_id;
  u16 resv_blocks_group_id;

  // extended sb
  u32 first_inode;
  u16 inode_size;
  u16 sb_bgd;
  u32 opt_features;
  u32 req_features; // features needed to mount
  u32 mount_features; // features needed to mount as read-only
  u8 fs_id[16];
  char vol_name[16];
  char vol_path_mount[64];
  u32 compression_algo;
  u8 preallocate_blocks_file;
  u8 preallocate_blocks_dir;
  u16 unused;
  u64 journal_id[2];
  u32 journal_inode;
  u32 journal_device;
  u32 orphan_inode_list;
  u8 unused_ext[787];
} ext2_sb;

typedef struct {
  u16 type_perms;
  u16 user_id;
  u32 size_low;
  u32 last_access_time;
  u32 creation_time;
  u32 mod_time;
  u32 deletion_time;
  u16 group_id;
  u16 hard_link_count;
  u32 sector_count;
  u32 flags;
  u32 os_spec;
  u32 direct_block_ptr[12];
  u32 singly_block_ptr;
  u32 doubly_block_ptr;
  u32 triply_block_ptr;
  u32 gen_number;
  u32 attrib_block;
  u32 size_high;
  u32 frag_block_addr;
  u8 os_spec2[12];
} ext2_inode;

typedef struct {
  u32 inode;
  u16 total_size;
  u8 name_len;
  u8 type;
  //only if the feature bit for "directory entries have file type byte" is set, else this is the most-significant 8 bits of the Name Length
  u8 name[];
} ext2_dirent;

typedef struct {
  u32 bitmap_block;
  u32 bitmap_inode;
  u32 inode_table_block;
  u16 free_blocks;
  u16 free_inodes;
  u16 directories_count;
  u16 pad;
  u8 resv[12];
} ext2_bgd;

typedef struct {
  ext2_sb* sb;
  ext2_bgd* bgd_table;
  u32 block_size;
  u32 bgd_count;
  u32 bgd_block;
  u32 inode_size;
} ext2_fs;

u8 ext2_init();