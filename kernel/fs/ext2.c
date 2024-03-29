
/*
  EXT2 filesystem implementation.

  references:
    * https://engineering.purdue.edu/~ee469/lectures/469_lec22_part3.pdf
    * https://web.archive.org/web/20211027105440/https://wiki.osdev.org/Ext2
    * http://www.science.smith.edu/~nhowe/Teaching/csc262/oldlabs/ext2.html
*/

#include "types.h"
#include "file.h"
#include "stat.h"
#include "fs/ext2.h"
#include "buf.h"
#include "defs.h"
#include "kernel/string.h"
#include "spinlock.h"


struct ext2_superblock sb;
#define DISK_SECTOR_SIZE 512

extern uint64 ROOT_DEV;

static inline void readblock(uint16 devt, uint32 devnum, uint64 blocknum, uint32 blocksize, void *buf, int n);

uint8 ext2_init_dev(uint16 devt, uint32 devnum) {
    ext2_readsb(devt, devnum, &sb);
    if (sb.ext2_signature != FS_EXT2_SIGNATURE) {
        return 0;
    }
    int blocksize = (1024 << sb.block_size);
    if (blocksize > FS_EXT2_MAX_BLOCK_SIZE) {
        cprintf("unsupported block size (%d bytes) - unmountable\n", blocksize);
        return 0;
    }
    return 1;
}

void ext2_readsb(uint16 devt, uint32 devnum, struct ext2_superblock* sb2) {
    readblock(devt, devnum, 1, FS_EXT2_SB_SIZE, &sb, FS_EXT2_SB_SIZE);
    if (&sb != sb2) {
        memcopy(sb2, &sb, sizeof(sb));
    }
}

void ext2_ilock(struct inode *ip) {
    panic("ext2_ilock not implemented yet");
}

int ext2_readi(struct inode *ip, char *dst, uint off, uint n) {
    struct ext2_blockgroupdesc bgd;
    uint8 devt = GETDEVTYPE(ip->dev);
    uint32 devnum = GETDEVNUM(ip->dev);

    readblock(devt, devnum, 2, sb.block_size, &bgd, sizeof(bgd));

    int blocksize = (1024 << sb.block_size);
    // int blockgroup = (ip->inum - 1) / sb.inodes_in_group;
    int index = (ip->inum - 1) % sb.inodes_in_group;
    int inodesize = sb.major_ver >= 1 ? sb.inode_size : FS_EXT2_OLD_INODE_SIZE;
    int containingblock = (index * inodesize) / blocksize;

    char *buf = kalloc();
    readblock(devt, devnum, containingblock + 3, sb.block_size, buf, sb.block_size);
    memmove(dst, buf + (index * inodesize), inodesize);
    kfree(buf);
    return 1;
}

int ext2_writei(struct inode *ip, char *src, uint off, uint n) {
    panic("ext2_writei not implemented yet");
}

int ext2_dirlink(struct inode *dp, char *name, uint inum) {
    panic("ext2_dirlink not implemented yet");
}

struct inode *ext2_dirlookup(struct inode *dp, char *name, uint *poff) {
    panic("ext2_dirlookup not implemented yet");
}

struct inode *ext2_ialloc(uint dev, short type) {
    panic("ext2_ialloc not implemented yet");
}

struct inode *ext2_idup(struct inode *ip) {
    panic("ext2_idup not implemented yet");
}

void ext2_iupdate(struct inode *ip) {
    panic("ext2_iupdate not implemented yet");
}

int ext2_namecmp(const char *s, const char *t) {
    panic("ext2_namecmp not implemented yet");
}

void ext2_stati(struct inode *ip, struct stat *st) {
    panic("ext2_stati not implemented yet");
}

void ext2_itrunc(struct inode* ip) {
    panic("ext2_itrunc not implemented yet");
}

void dump_data(uint8 *ptr, int cnt) {
    int j = 0;
    while(j <= cnt) {
        for(int i = 0; i != 60; i++) {
            cprintf("%x ", ptr[j++]);
        }
        cprintf("\n");
    }
}

void ext2_readinode(struct inode *ip) {
    struct buf *bp;
	struct ext2_inode *ext2i;

    int blocksize = (1024 << sb.block_size);
    int index = (ip->inum - 1) % sb.inodes_in_group;
    int inodesize = sb.major_ver >= 1 ? sb.inode_size : FS_EXT2_OLD_INODE_SIZE;
    int block = (index * inodesize) / blocksize;
    cprintf("About to read block %d\n", block);

    bp = bread(ip->dev, block + 2);
    dump_data(bp->data, 4096);
    cprintf("Seeking to inode @ index %d\n", index);
    ext2i = (struct ext2_inode*)bp->data + (index * sizeof(struct ext2_inode));
    dump_data((uint8 *)ext2i, sizeof(struct ext2_inode));
    if (EXT2_TYPE_ISREG(ext2i->type)) {
        ip->type = T_FILE;
    } else if (EXT2_TYPE_ISDIR(ext2i->type)) {
        ip->type = T_DIR;
    } else {
        cprintf("Uknown type %d\n", ext2i->type);
        ip->type = 0; // unknown!
    }
    ip->nlink = ext2i->hard_link_count;
    ip->size = ext2i->lower_size;
    memmove(ip->addrs, ext2i->block_pointers, sizeof(ext2i->block_pointers));
    brelse(bp);
}

static inline void readblock(uint16 devt, uint32 devnum, uint64 blocknum, uint32 blocksize, void *buf, int n) {
    uint32 spb = blocksize / DISK_SECTOR_SIZE; // 2
    uint64 start = blocknum * spb;
    uint32 legacydevid = TODEVNUM(devt, devnum);

    for(uint8 i = 0; i != spb && n > 0; i++) {
        struct buf* bp = bread(legacydevid, start + i);
        void *offset = buf + (DISK_SECTOR_SIZE * i);
        memcopy(offset, &bp->data[0], n > DISK_SECTOR_SIZE ? DISK_SECTOR_SIZE : 0);
        n -= DISK_SECTOR_SIZE;
        brelse(bp);
    }
}
