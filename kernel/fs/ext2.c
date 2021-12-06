#include "types.h"
#include "file.h"
#include "fs/ext2.h"
#include "buf.h"
#include "defs.h"
#include "kernel/string.h"


struct ext2_superblock sb;
#define DISK_SECTOR_SIZE 512

static inline void readblock(uint16 devt, uint32 devnum, uint64 blocknum, uint32 blocksize, void *buf) {
    uint32 spb = blocksize / DISK_SECTOR_SIZE; // 2
    uint64 start = blocknum * spb;
    uint32 legacydevid = TODEVNUM(devt, devnum);

    for(uint8 i = 0; i != spb; i++) {
        struct buf* bp = bread(legacydevid, start + i);
        void *offset = buf + (DISK_SECTOR_SIZE * i);
        memcopy(offset, &bp->data[0], DISK_SECTOR_SIZE);
        brelse(bp);
    }
}

uint8 ext2_init_dev(uint16 devt, uint32 devnum) {
    ext2_readsb(devt, devnum, &sb);
    return (sb.ext2_signature == FS_EXT2_SIGNATURE) ? 1 : 0;
}

void ext2_readsb(uint16 devt, uint32 devnum, struct ext2_superblock* sb2) {
    readblock(devt, devnum, 1, 1024, &sb);
    if (&sb != sb2) {
        memcopy(sb2, &sb, sizeof(sb));
    }
}

int ext2_readi(struct inode *ip, char *dst, uint off, uint n) {
  // TODO
  return 0;
}
