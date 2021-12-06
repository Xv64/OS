#include "types.h"
#include "file.h"
#include "fs/ext2.h"
#include "buf.h"
#include "defs.h"
#include "kernel/string.h"


struct ext2_superblock sb;

uint8 ext2_init_dev(uint16 devt, uint32 devnum) {
    ext2_readsb(devt, devnum, &sb);

    return (sb.ext2_signature == FS_EXT2_SIGNATURE) ? 1 : 0;
}

void ext2_readsb(uint16 devt, uint32 devnum, struct ext2_superblock* sb2) {
    // assumes 512 byte sectors...
    struct buf* bp1 = bread(TODEVNUM(devt, devnum), 2);
    struct buf* bp2 = bread(TODEVNUM(devt, devnum), 3);

    memcopy(&sb, &bp1->data[0], 512);
    void *offset = &sb;
    offset += 512;
    memcopy(offset, &bp2->data[0], 512);

    brelse(bp1);
    brelse(bp2);

    memcopy(sb2, &sb, sizeof(sb));
}

int ext2_readi(struct inode *ip, char *dst, uint off, uint n) {
  // TODO
  return 0;
}
