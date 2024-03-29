// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "kernel/string.h"

void freerange(void* vstart, void* vend);
extern char end[]; // first address after kernel loaded from ELF file

struct run {
	struct run* next;
};

struct {
	struct spinlock lock;
	int use_lock;
	struct run* freelist;
} kmem;

int kalloc_fullysetup = 0;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void kinit1(void* vstart, void* vend){
	uint64 startmb = V2P(vstart) / 1024 / 1024;
	uint64 endmb = V2P(vend) / 1024 / 1024;
	cprintf("Freeing mem from %d MB to %d MB...\n", startmb, endmb);
	initlock(&kmem.lock, "kmem");
	kmem.use_lock = 0;
	freerange(vstart, vend);
}

void kinit2(void* vstart, void* vend){
	uint64 startmb = V2P(vstart) / 1024 / 1024;
	uint64 endmb = V2P(vend) / 1024 / 1024;
	cprintf("Freeing mem from %d MB to %d MB...\n", startmb, endmb);
	freerange(vstart, vend);
	kmem.use_lock = 1;
	kalloc_fullysetup = 1;
}

void freerange(void* vstart, void* vend){
	char* p;
	p = (char*)PGROUNDUP((uintp)vstart);
	for (; p + PGSIZE <= (char*)vend; p += PGSIZE)
		kfree(p);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(char* v){
	struct run* r;

	if ((uintp)v % PGSIZE || v < end || v2p(v) >= PHYSTOP)
		panic("kfree");

	// Fill with junk to catch dangling refs.
	memset(v, 1, PGSIZE);

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = (struct run*)v;
	r->next = kmem.freelist;
	kmem.freelist = r;
	if (kmem.use_lock)
		release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char* kalloc(void){
	struct run* r;

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = kmem.freelist;
	if (r)
		kmem.freelist = r->next;
	if (kmem.use_lock)
		release(&kmem.lock);
	return (char*)r;
}

char *kmalloc(uint16 pages){
	if(!kalloc_fullysetup) {
		panic("kalloc not fully setup");
	}
	struct run* r;

	if (kmem.use_lock)
		acquire(&kmem.lock);
	r = kmem.freelist;
	if (r) {
		struct run* n = r->next;
		for(uint8 i = 0; i != pages; i++) {
			kmem.freelist = n;
			n = n->next;
		}
	}
	if (kmem.use_lock)
		release(&kmem.lock);
	return (char*)r;
}
