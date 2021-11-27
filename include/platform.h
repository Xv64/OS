#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#define byteswap16(v) ((v & 0x00ff) << 8 | (v & 0xff00 ) >> 8)
#define byteswap32(v) ((v & 0x000000ff) << 24 | (v & 0x0000ff00) << 8 | (v & 0x00ff0000) >> 8 | (v & 0xff000000) >> 24)

int byteorder (void); // defined in klib.c
