#define FS_MAX_SB_SIZE 4096

// generic super block
struct superblock {
  uint8 data[FS_MAX_SB_SIZE];
};
