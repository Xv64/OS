// On-disk file system format.
// Both the kernel and user programs use this header file.

// Block 0 is unused.
// Block 1 is super block.
// Blocks 2 through sb.ninodes/IPB hold inodes.
// Then free bitmap blocks holding sb.size bits.
// Then sb.nblocks data blocks.
// Then sb.nlog log blocks.

#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

struct fs1_superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
};

void            fs1_readsb(int dev, struct fs1_superblock *sb);
int             fs1_dirlink(struct inode*, char*, uint);
struct inode*   fs1_dirlookup(struct inode*, char*, uint*);
struct inode*   fs1_ialloc(uint, short);
struct inode*   fs1_idup(struct inode*);
void            fs1_iinit(void);
void            fs1_ilock(struct inode*);
void            fs1_iput(struct inode*);
void            fs1_iunlock(struct inode*);
void            fs1_iunlockput(struct inode*);
void            fs1_iupdate(struct inode*);
int             fs1_namecmp(const char*, const char*);
struct inode*   fs1_namei(char*);
struct inode*   fs1_nameiparent(char*, char*);
int             fs1_readi(struct inode*, char*, uint, uint);
void            fs1_stati(struct inode*, struct stat*);
int             fs1_writei(struct inode*, char*, uint, uint);

#define NDIRECT 28
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  int16  type;               // File type
  int16  major;              // Major device number (T_DEV only)
  int16  minor;              // Minor device number (T_DEV only)
  int16  nlink;              // Number of links to inode in file system
  uint32 size;               // Size of file (bytes)
  uint32 addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};
