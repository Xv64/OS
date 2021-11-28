struct ext2_superblock {
    uint32 inode_count;           // Total number of inodes in file system
    uint32 block_count;           // Total number of blocks in file system
    uint32 superuser_block_count; // Number of blocks reserved for superuser (see offset 80)
    uint32 unallocated_blockcnt;  // Total number of unallocated blocks
    uint32 unallocated_inodecnt;  // Total number of unallocated inodes
    uint32 superblock_num;        // Block number of the block containing the superblock
    uint32 block_size;            // log2 (block size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the block size)
    uint32 fragment_size;         // log2 (fragment size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the fragment size)
    uint32 blocks_in_group;       // Number of blocks in each block group
    uint32 fragments_in_group;    // Number of fragments in each block group
    uint32 inodes_in_group;       // Number of inodes in each block group
    uint32 mount_time;            // Last mount time (in POSIX time)
    uint32 write_time;            // Last written time (in POSIX time)
    uint16 mount_count;           // Number of times the volume has been mounted since its last consistency check (fsck)
    uint16 max_mount_count;       // Number of mounts allowed before a consistency check (fsck) must be done
    uint16 ext2_signature;        // Ext2 signature (0xef53), used to help confirm the presence of Ext2 on a volume
    uint16 fs_state;              // File system state
    uint16 error_cnd;             // What to do when an error is detected
    uint16 minor_ver;             // Minor portion of version (combine with Major portion below to construct full version field)
    uint32 last_chck;             // POSIX time of last consistency check (fsck)
    uint32 max_chck_duration;     // Interval (in POSIX time) between forced consistency checks (fsck)
    uint32 system_id;             // Operating system ID from which the filesystem on this volume was created
    uint32 major_ver;             // Major portion of version (combine with Minor portion above to construct full version field)
    uint16 reserved_uid;          // User ID that can use reserved blocks
    uint16 reserved_gid;          // Group ID that can use reserved blocks
}; // 1204 bytes in total

#define FS_EXT2_SIGNATURE 0xEF53

// File System States:
#define FS_EXT2_CLEAN  0x1
#define FS_EXT2_ERRORS 0x2

// Error Handling Methods:
#define FS_EXT2_IGNORE_ERROR 0x1
#define FS_EXT2_REMOUNT_RO   0x2
#define FS_EXT2_PANIC_ON_ERR 0x3

// Creator Operating System IDs
#define FS_EXT2_CREATOR_LINUX   0x0
#define FS_EXT2_CREATOR_TURD    0x1
#define FS_EXT2_CREATOR_MASIX   0x2
#define FS_EXT2_CREATOR_FREEBSD 0x3
#define FS_EXT2_CREATOR_OTHER   0x4

uint8 init_dev(uint16 devt, uint32 devnum);
