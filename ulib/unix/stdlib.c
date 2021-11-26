#include "syscalls.h"
#include "unix/ctype.h"
#include "unix/stdint.h"
#include "unix/limits.h"
#include "unix/string.h"

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
	int i = 0;
	int multiplier = 1;
	while(str[i] != '\0' && isspace(str[i]) ) {
		i++;
	}
	if(str[i] != '\0' && (str[i] == '+' || str[i] == '-')) {
		if(str[i] == '-') {
			multiplier = -1;
		}
		i++;
	}
	while(str[i] != '\0') {
		int value = ((int)str[i]) - 0x30;
		if(value < 0 || value > 9) {
			break;
		}
		result = result * 10 + value; //shift existing value over and add new digit
		i++;
	}
	return result * multiplier;
}

void exit(int status) {
	procexit();
}

long strtol(const char *restrict str, char **restrict endptr, int base) {
	//POSIX Base Definitions, Issue 6 - page 1456
	const char *p = str;

	while(isspace( (int)*p )) {
		p++;
	}

	int n = 0;
	while(isdigit( (int)*p )) {
		n = n*10 + (int)*p - '0';
		p++;
	}
	endptr = (char **) &p;
	return n;
}
