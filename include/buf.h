#define SECTOR_SIZE 512

struct buf {
  int32 flags;
  uint32 dev;
  uint32 sector;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk queue
  uint8 data[SECTOR_SIZE];
};
#define B_BUSY  0x1  // buffer is locked by some process
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

#define DEV_TYPE_MASK 0xF0000000
#define DEV_NUM_MASK  0x0FFFFFFF
#define DEV_IDE 0
#define DEV_SATA 1

#define GETDEVTYPE(a) ((a & DEV_TYPE_MASK) >> 28)
#define GETDEVNUM(a) (a & DEV_NUM_MASK)
#define TODEVNUM(type, num) ((type << 28) + (DEV_NUM_MASK & num))
