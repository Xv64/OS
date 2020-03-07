typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

//explicit-length unsigned ints
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef unsigned long uint64;

//explicit-length signed ints
typedef char int8;
typedef short int16;
typedef int  int32;
typedef long int64;

#if X64
typedef unsigned long uintp;
#else
typedef unsigned int  uintp;
#endif

typedef uintp pde_t;
