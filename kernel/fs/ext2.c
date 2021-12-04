#include "types.h"
#include "fs/ext2.h"
#include "buf.h"
#include "defs.h"
#include "kernel/string.h"


struct ext2_superblock sb;

uint8 ext2_init_dev(uint16 devt, uint32 devnum) {
    // assumes 512 byte sectors...

    struct buf* bp1 = bread(TODEVNUM(devt, devnum), 2);
    struct buf* bp2 = bread(TODEVNUM(devt, devnum), 3);

    memcopy(&sb, &bp1->data[0], 512);
    void *offset = &sb;
    offset += 512;
    memcopy(offset, &bp2->data[0], 512);

    brelse(bp1);
    brelse(bp2);

    return (sb.ext2_signature == FS_EXT2_SIGNATURE) ? 1 : 0;
}
