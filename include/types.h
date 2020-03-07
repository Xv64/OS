typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;

#if X64
typedef unsigned long uintp;
#else
typedef unsigned int  uintp;
#endif

typedef uintp pde_t;
