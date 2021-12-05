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
