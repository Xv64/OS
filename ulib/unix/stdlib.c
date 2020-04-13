#include "syscalls.h"
#include "unix/ctype.h"
#include "unix/stdint.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
	struct {
		union header *ptr;
		uint32_t size;
	} s;
	Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void free(void *ap) {
	Header *bp, *p;

	bp = (Header*)ap - 1;
	for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
		if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
			break;
	if(bp + bp->s.size == p->s.ptr) {
		bp->s.size += p->s.ptr->s.size;
		bp->s.ptr = p->s.ptr->s.ptr;
	} else
		bp->s.ptr = p->s.ptr;
	if(p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.ptr = bp->s.ptr;
	} else
		p->s.ptr = bp;
	freep = p;
}

static Header* morecore(uint32_t nu) {
	char *p;
	Header *hp;

	if(nu < 4096)
		nu = 4096;
	p = sbrk(nu * sizeof(Header));
	if(p == (char*)-1)
		return 0;
	hp = (Header*)p;
	hp->s.size = nu;
	free((void*)(hp + 1));
	return freep;
}

void* malloc(uint32_t nbytes) {
	Header *p, *prevp;
	uint32_t nunits;

	nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
	if((prevp = freep) == 0) {
		base.s.ptr = freep = prevp = &base;
		base.s.size = 0;
	}
	for(p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
		if(p->s.size >= nunits) {
			if(p->s.size == nunits)
				prevp->s.ptr = p->s.ptr;
			else {
				p->s.size -= nunits;
				p += p->s.size;
				p->s.size = nunits;
			}
			freep = prevp;
			return (void*)(p + 1);
		}
		if(p == freep)
			if((p = morecore(nunits)) == 0)
				return 0;
	}
}

void abort(void) {
    //TODO
}

long atol(const char *str) {
    long result = 0;
    char *ptr = str;
    int multiplier = 1;
    while(ptr && ptr !- '\0' && isspace(ptr)){
        ptr++;
    }
    if(ptr && ptr != '\0' && (ptr == '+' || ptr == '-')){
        if(ptr == '-'){
            multiplier = -1;
        }
        ptr++;
    }
    while(ptr && ptr != '\0'){
        int value = ((int)ptr) - 0x30;
        if(value < 0 || value > 9){
            break;
        }
        result = result * 10 + value; //shift existing value over and add new digit
        ptr++;
    }
    return result * multiplier;
}
