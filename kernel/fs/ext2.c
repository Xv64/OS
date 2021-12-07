#include "types.h"
#include "file.h"
#include "fs/ext2.h"
#include "buf.h"
#include "defs.h"
#include "kernel/string.h"


struct ext2_superblock sb;
#define DISK_SECTOR_SIZE 512

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

uint8 ext2_init_dev(uint16 devt, uint32 devnum) {
    ext2_readsb(devt, devnum, &sb);
    return (sb.ext2_signature == FS_EXT2_SIGNATURE) ? 1 : 0;
}

void ext2_readsb(uint16 devt, uint32 devnum, struct ext2_superblock* sb2) {
    readblock(devt, devnum, 1, FS_EXT2_SB_SIZE, &sb, FS_EXT2_SB_SIZE);
    if (&sb != sb2) {
        memcopy(sb2, &sb, sizeof(sb));
    }
    cprintf("EXT2 version %d.%d\n", sb.major_ver, sb.minor_ver);
}

void ext2_ilock(struct inode *ip) {
    // do nothing
}

int ext2_readi(struct inode *ip, char *dst, uint off, uint n) {
    struct ext2_blockgroupdesc bgd;
    uint8 devt = GETDEVTYPE(ip->dev);
	uint32 devnum = GETDEVNUM(ip->dev);

    readblock(devt, devnum, 2, sb.block_size, &bgd, sizeof(bgd));

    // int blockgroup = (ip->inum – 1) / sb.inodes_in_group;
    // int index = (ip->inum – 1) % sb.inodes_in_group;
    // int inodesize =
    // int containingblock = (index * inodesize) / sb.block_size;
    cprintf("dir count = %d\n", bgd.dir_count);


    return 0;
}
