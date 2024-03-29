// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define AHCI_MEM 0x500000           // 5 MB to 36 MB
#define KALLOC_START 0x3200000      // start kalloc @ 50 MB
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0xFFFFFFFF80000000 // First kernel virtual address
#define DEVBASE  0xFFFFFFFF40000000 // First device virtual address
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

#ifndef __ASSEMBLER__

static inline uintp v2p(void *a) { return ((uintp) (a)) - ((uintp)KERNBASE); }
static inline void *p2v(uintp a) { return (void *) ((a) + ((uintp)KERNBASE)); }

#endif

#define V2P(a) (((uintp) (a)) - KERNBASE)
#define P2V(a) (((void *) (a)) + KERNBASE)
#define IO2V(a) (((void *) (a)) + DEVBASE - DEVSPACE)

#define ADDRHI(a) ((a >> 32) & 0xffffffff)
#define ADDRLO(a) (a & 0xffffffff)
#define HILO2ADDR(hi, lo) ((((uint64)hi) << 32) + lo)

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as V2P, but without casts
