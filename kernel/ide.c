// Simple PIO-based (non-DMA) IDE driver code.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "buf.h"

#define IDE_BSY       0x80
#define IDE_DRDY      0x40
#define IDE_DF        0x20
#define IDE_ERR       0x01

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30

#define PRIMARY_IDE_CHANNEL_BASE   0x1F0
#define PRIMARY_IDE_INTERRUPT      0x3F6
#define SECONDARY_IDE_CHANNEL_BASE 0x170
#define SECONDARY_IDE_INTERRUPT    0x376

#define IDE_MASTER (0xe0 | (0 << 4))
#define IDE_SLAVE  (0xe0 | (1 << 4))

// idequeue points to the buf now being read/written to the disk.
// idequeue->qnext points to the next buf to be processed.
// You must hold idelock while manipulating queue.

static struct spinlock idelock;
static struct buf* idequeue;
uint16 ideChannel = SECONDARY_IDE_CHANNEL_BASE;

static int havedisk1;
static void idestart(struct buf*);

// Wait for IDE disk to become ready.
static int idewait(int checkerr){
    int r;

    while (((r = inb(PRIMARY_IDE_CHANNEL_BASE + 7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY)
        ;
    if (checkerr && (r & (IDE_DF | IDE_ERR)) != 0)
        return -1;
    return 0;
}

void ideinit(void){
    int i;

    initlock(&idelock, "ide");
    if(ideChannel == PRIMARY_IDE_CHANNEL_BASE){
        picenable(IRQ_IDE1);
        ioapicenable(IRQ_IDE1, ncpu - 1);
    }else{
        picenable(IRQ_IDE2);
        ioapicenable(IRQ_IDE2, ncpu - 1);
    }
    idewait(0);

    // Check if disk 1 is present
    amd64_out8(ideChannel + 6, IDE_SLAVE);
    for (i = 0; i < 1000; i++) {
        if (inb(ideChannel + 7) != 0) {
            havedisk1 = 1;
            break;
        }
    }

    // Switch back to disk 0.
    amd64_out8(ideChannel + 6, IDE_MASTER);
}

// Start the request for b.  Caller must hold idelock.
static void idestart(struct buf* b){
    if (b == 0)
        panic("idestart");

    idewait(0);
    amd64_out8((ideChannel == PRIMARY_IDE_CHANNEL_BASE ? PRIMARY_IDE_INTERRUPT : SECONDARY_IDE_INTERRUPT), 0); // generate interrupt
    amd64_out8(ideChannel + 2, 1); // number of sectors
    amd64_out8(ideChannel + 3, b->sector & 0xff);
    amd64_out8(ideChannel + 4, (b->sector >> 8) & 0xff);
    amd64_out8(ideChannel + 5, (b->sector >> 16) & 0xff);
    amd64_out8(ideChannel + 6, (b->dev == 1 ? IDE_SLAVE : IDE_MASTER) | ((b->sector >> 24) & 0x0f));
    if (b->flags & B_DIRTY) {
        amd64_out8(ideChannel + 7, IDE_CMD_WRITE);
        outsl(ideChannel, b->data, 512 / 4);
    } else {
        amd64_out8(ideChannel + 7, IDE_CMD_READ);
    }
}

// Interrupt handler.
void ideintr(void){
    struct buf* b;

    // First queued buffer is the active request.
    acquire(&idelock);
    if ((b = idequeue) == 0) {
        release(&idelock);
        // cprintf("spurious IDE interrupt\n");
        return;
    }
    idequeue = b->qnext;

    // Read data if needed.
    if (!(b->flags & B_DIRTY) && idewait(1) >= 0)
        insl(ideChannel, b->data, 512 / 4);

    // Wake process waiting for this buf.
    b->flags |= B_VALID;
    b->flags &= ~B_DIRTY;
    wakeup(b);

    // Start disk on next buf in queue.
    if (idequeue != 0)
        idestart(idequeue);

    release(&idelock);
}

// Sync buf with disk.
// If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
// Else if B_VALID is not set, read buf from disk, set B_VALID.
void iderw(struct buf* b){
    struct buf** pp;

    if (!(b->flags & B_BUSY))
        panic("iderw: buf not busy");
    if ((b->flags & (B_VALID | B_DIRTY)) == B_VALID)
        panic("iderw: nothing to do");
    if (b->dev != 0 && !havedisk1)
        panic("iderw: ide disk 1 not present");

    acquire(&idelock); //DOC:acquire-lock

    // Append b to idequeue.
    b->qnext = 0;
    for (pp = &idequeue; *pp; pp = &(*pp)->qnext) //DOC:insert-queue
        ;
    *pp = b;

    // Start disk if necessary.
    if (idequeue == b)
        idestart(b);

    // Wait for request to finish.
    while ((b->flags & (B_VALID | B_DIRTY)) != B_VALID) {
        sleep(b, &idelock);
    }

    release(&idelock);
}
