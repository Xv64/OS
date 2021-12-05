#define FS_MAX_SB_SIZE 4096
#define FS_MAX_INIT_DEV 1

// generic super block
struct superblock {
  uint8 data[FS_MAX_SB_SIZE];
};

typedef int16 fstype;

#define FS_TYPE_FS1  0x1
#define FS_TYPE_EST2 0x2
