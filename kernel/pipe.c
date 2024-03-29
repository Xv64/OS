#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "vfs.h"
#include "file.h"
#include "spinlock.h"
#include "x86.h"
#include "fcntl.h"

#define PIPESIZE 512
#define LOCK_WAIT_TICKS 100
#define MAX_WRITE_WAIT 1000
#define MAX_READ_WAIT  1000

struct pipe {
	struct spinlock lock;
	char data[PIPESIZE];
	uint nread; // number of bytes read
	uint nwrite; // number of bytes written
	int readopen; // read fd is still open
	int writeopen; // write fd is still open
};

int pipealloc(struct file** f0, struct file** f1){
	struct pipe* p;

	p = 0;
	*f0 = *f1 = 0;
	if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
		goto bad;
	if ((p = (struct pipe*)kalloc()) == 0)
		goto bad;
	p->readopen = 1;
	p->writeopen = 1;
	p->nwrite = 0;
	p->nread = 0;
	initlock(&p->lock, "pipe");
	(*f0)->type = FD_PIPE;
	(*f0)->readable = 1;
	(*f0)->writable = 0;
	(*f0)->pipe = p;
	(*f1)->type = FD_PIPE;
	(*f1)->readable = 0;
	(*f1)->writable = 1;
	(*f1)->pipe = p;
	return 0;

bad:
	if (p)
		kfree((char*)p);
	if (*f0)
		fileclose(*f0);
	if (*f1)
		fileclose(*f1);
	return -1;
}

void pipeclose(struct pipe* p, int writable){
	acquire(&p->lock);
	if (writable) {
		p->writeopen = 0;
		wakeup(&p->nread);
	} else {
		p->readopen = 0;
		wakeup(&p->nwrite);
	}
	if (p->readopen == 0 && p->writeopen == 0) {
		release(&p->lock);
		kfree((char*)p);
	} else
		release(&p->lock);
}

int pipewrite(struct pipe* p, char* addr, int n){
	int i;

	uint8 success = sacquire(&p->lock, LOCK_WAIT_TICKS);
	if(success != SPINLOCK_ACQUIRED) {
		return FNOT_READY;
	}
	for (i = 0; i < n; i++) {
		uint32 loops = 0;
		while ((p->nwrite == p->nread + PIPESIZE) && loops++ < MAX_WRITE_WAIT) { // pipewrite-full
			if (p->readopen == 0 || proc->killed) {
				release(&p->lock);
				return F_ERROR;
			}
			wakeup(&p->nread);
			amd64_nop();
		}
		if(loops >= MAX_WRITE_WAIT) {
			wakeup(&p->nread);
			release(&p->lock);
			return FNOT_READY;
		}
		p->data[p->nwrite++ % PIPESIZE] = addr[i];
	}
	wakeup(&p->nread); // pipewrite-wakeup1
	release(&p->lock);
	return n;
}

int piperead(struct pipe* p, char* addr, int n){
	int i;

	uint8 success = sacquire(&p->lock, LOCK_WAIT_TICKS);
	if(success != SPINLOCK_ACQUIRED) {
		return FNOT_READY;
	}
	uint32 loops = 0;
	while (p->nread == p->nwrite && p->writeopen && loops++ < MAX_READ_WAIT) { // pipe-empty
		if (proc->killed) {
			release(&p->lock);
			return F_ERROR;
		}
		amd64_nop();
	}
	if(loops >= MAX_READ_WAIT) {
		wakeup(&p->nwrite);
		release(&p->lock);
		return FNOT_READY;
	}
	for (i = 0; i < n; i++) { // piperead-copy
		if (p->nread == p->nwrite)
			break;
		addr[i] = p->data[p->nread++ % PIPESIZE];
	}
	wakeup(&p->nwrite); // piperead-wakeup
	release(&p->lock);
	return i;
}
