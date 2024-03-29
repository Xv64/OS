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
    // extended EXT2 fields (only available in version >= 1.0)...
    uint32 fst_nr_inode;          // First non-reserved inode in file system. (In versions < 1.0, this is fixed as 11)
    uint16 inode_size;            // Size of each inode structure in bytes. (In versions < 1.0, this is fixed as 128)
    uint16 sb_block_group;        // Block group that this superblock is part of (if backup copy)
    uint32 optional_features;     // Optional features present (features that are not required to read or write, but usually result in a performance increase. see below)
    uint32 required_features;     // Required features present (features that are required to be supported to read or write. see below)
    uint32 required_or_ro;        // Features that if not supported, the volume must be mounted read-only see below)
    uint8  fsid[16];              // File system ID (what is output by blkid)
    uint8  fsname[16];            // Volume name (C-style string: characters terminated by a 0 byte)
    uint8  last_mount_path[64];   // Path volume was last mounted to (C-style string: characters terminated by a 0 byte)
    uint32 compression_algo;      // Compression algorithms used (see Required features above)
    uint8  preallocated_files;    // Number of blocks to preallocate for files
    uint8  preallocated_dirs;     // Number of blocks to preallocate for directories
    uint8  unused[2];
    uint8  journalid[16];         // Journal ID (same style as the File system ID above)
    uint32 journal_inode;         // Journal inode
    uint32 journal_dev;           // Journal device
    uint32 orphan_head;           // Head of orphan inode list
    uint8  reserved[788];

}; // 1204 bytes in total

struct ext2_blockgroupdesc {
    uint32 block_usage_addr;      // Block address of block usage bitmap
    uint32 inode_usage_addr;      // Block address of inode usage bitmap
    uint32 inode_tbl_addr;        // Starting block address of inode table
    uint16 unallocated_blocks;    // Number of unallocated blocks in group
    uint16 unallocated_inodes;    // Number of unallocated inodes in group
    uint16 dir_count;             // Number of directories in group
    uint32 reversed[3];
};

struct ext2_inode {
    uint16 type;                   // Type and Permissions (see below)
    uint16 user_id;
    uint32 lower_size;             // Lower 32 bits of size in bytes
    uint32 last_accessed_at;       // Last Access Time (in POSIX time)
    uint32 creation_time;          // Creation Time (in POSIX time)
    uint32 created_at;             // Last Modification time (in POSIX time)
    uint32 deleted_at;             // Deletion time (in POSIX time)
    uint16 group_id;
    uint16 hard_link_count;        // Count of hard links (directory entries) to this inode. When this reaches 0, the data blocks are marked as unallocated.
    uint32 disk_sectors;           // Count of disk sectors (not Ext2 blocks) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode
    uint32 flags;                  // Flags (see below)
    uint32 os_value;               // Operating System Specific value #1 (see https://web.archive.org/web/20211027105440/https://wiki.osdev.org/Ext2#OS_Specific_Value_1)
    uint32 block_pointers[12];     // Direct Block Pointers 0 - 11
    uint32 indirect_blk_ptr;       // Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)
    uint32 dbl_indirect_blk_ptr;   // Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)
    uint32 triple_indirect_blk_ptr;// Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)
    uint32 gen_num;                // Generation number (Primarily used for NFS)
    uint32 file_acl;               // In Ext2 version 0, this field is reserved. In version >= 1, Extended attribute block (File ACL).
    uint32 dir_acl;                // In Ext2 version 0, this field is reserved. In version >= 1, Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory
    uint32 fragment_blk_addr;      // Block address of fragment
    uint32 os_value2[3];           // Operating System Specific Value #2 (see https://web.archive.org/web/20211027105440/https://wiki.osdev.org/Ext2#OS_Specific_Value_2)
}; // 128 bytes

#define EXT2_IFMT  00170000
#define EXT2_IFREG  0100000
#define EXT2_IFDIR  0040000

#define EXT2_TYPE_ISREG(m)	(((m) & EXT2_IFMT) == EXT2_IFREG)
#define EXT2_TYPE_ISDIR(m)	(((m) & EXT2_IFMT) == EXT2_IFDIR)

#define FS_EXT2_SIGNATURE 0xEF53
#define FS_EXT2_SB_SIZE 1024
#define FS_EXT2_OLD_INODE_SIZE 128
#define FS_EXT2_MAX_BLOCK_SIZE 4096 // this should match the page size (in bytes) as allocated by kalloc

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

uint8         ext2_init_dev(uint16 devt, uint32 devnum);
void          ext2_readsb(uint16 devt, uint32 devnum, struct ext2_superblock* sb2);
void          ext2_ilock(struct inode *ip);
int           ext2_readi(struct inode *ip, char *dst, uint off, uint n);
void          ext2_readinode(struct inode *ip);
int           ext2_writei(struct inode *ip, char *src, uint off, uint n);
int           ext2_dirlink(struct inode *dp, char *name, uint inum);
struct inode *ext2_dirlookup(struct inode *dp, char *name, uint *poff);
struct inode *ext2_ialloc(uint dev, short type);
struct inode *ext2_idup(struct inode *ip);
void          ext2_iupdate(struct inode *ip);
int           ext2_namecmp(const char *s, const char *t);
void          ext2_stati(struct inode *ip, struct stat *st);
void          ext2_itrunc(struct inode* ip);
