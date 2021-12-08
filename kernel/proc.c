#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "kernel/string.h"
#include "vfs.h"
#include "file.h"

struct ptable_node {
    struct proc proc;
    struct ptable_node *next;
};

struct {
	struct spinlock lock;
	struct ptable_node *head;
} ptable;

#define EACH_PTABLE_NODE struct ptable_node *node = ptable.head; node->next != 0; node = node->next

static struct proc* initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);
int _fork(int blessed);
void _allocpipe(struct proc* p);
void _deallocpipe(struct proc* p);

static void wakeup1(void* chan);
int growptable();

int procloopread(struct inode* ip, char* buf, int n){
	//cprintf("Reading: minor=%d, from proc = %d\n", ip->minor, proc->pid);
	struct proc *tp;
	struct proc *p;

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == ip->minor) {
			tp = p;
			break;
		}
	}
	release(&ptable.lock);

	if(tp) {
		return fileread(tp->rpipe, buf, n);
	}
	return 0;
}

int procloopwrite(struct inode* ip, char* buf, int n){
	//cprintf("Writing: minor=%d, from proc = %d\n", ip->minor, proc->pid);
	struct proc *tp;
	struct proc *p;

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == ip->minor) {
			tp = p;
			break;
		}
	}
	release(&ptable.lock);

	if(tp) {
		return filewrite(tp->wpipe, buf, n);
	}
	return 0;
}

void pinit(void){
	initlock(&ptable.lock, "ptable");
	growptable();
}

void procloopinit() {
	cprintf("init: process loop device\n");
	devsw[LOOP0].write = procloopwrite;
	devsw[LOOP0].read = procloopread;
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc* allocproc(void){
	struct proc* p;
	char* sp;

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->state == UNUSED) {
			goto found;
		}
	}
	if(growptable() > 0) {
			release(&ptable.lock);
			return allocproc();
	}
	release(&ptable.lock);
	return 0;

found:
	p->state = EMBRYO;
	p->pid = nextpid++;
	p->priority = PROC_DEFAULT_PRIORITY;
	release(&ptable.lock);

	// Allocate kernel stack.
	if ((p->kstack = kalloc()) == 0) {
		p->state = UNUSED;
		return 0;
	}
	sp = p->kstack + KSTACKSIZE;

	// Leave room for trap frame.
	sp -= sizeof *p->tf;
	p->tf = (struct trapframe*)sp;

	// Set up new context to start executing at forkret,
	// which returns to trapret.
	sp -= sizeof(uintp);
	*(uintp*)sp = (uintp)trapret;

	sp -= sizeof *p->context;
	p->context = (struct context*)sp;
	memset(p->context, 0, sizeof *p->context);
	p->context->eip = (uintp)forkret;
	return p;
}

// Set up first user process.
void userinit(void){
	struct proc* p;
	extern char _binary_out_initcode_start[], _binary_out_initcode_size[];

	p = allocproc();
	initproc = p;
	if ((p->pgdir = setupkvm()) == 0)
		panic("userinit: out of memory?");
	inituvm(p->pgdir, _binary_out_initcode_start, (uintp)_binary_out_initcode_size);
	p->sz = PGSIZE;
	memset(p->tf, 0, sizeof(*p->tf));
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
	p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
	p->tf->eflags = FL_IF;
	p->tf->esp = PGSIZE;
	p->tf->eip = 0; // beginning of initcode.S

	safestrcpy(p->name, "initcode", sizeof(p->name));
	p->cwd = namei("/");
	p->blessed = PROC_BLESSED;
	p->priority = PROC_MAX_PRIORITY;
	_allocpipe(p);

	p->state = RUNNABLE;
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n){
	uint sz;

	sz = proc->sz;
	if (n > 0) {
		if ((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
			return -1;
	} else if (n < 0) {
		if ((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
			return -1;
	}
	proc->sz = sz;
	switchuvm(proc);
	return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int _fork(int blessed){
	int i, pid;
	struct proc* np;

	// Allocate process.
	if ((np = allocproc()) == 0)
		return -1;

	// Copy process state from p.
	if ((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0) {
		kfree(np->kstack);
		np->kstack = 0;
		np->state = UNUSED;
		return -1;
	}
	np->sz = proc->sz;
	np->parent = proc;
	*np->tf = *proc->tf;

	// Clear %eax so that fork returns 0 in the child.
	np->tf->eax = 0;

	for (i = 0; i < NOFILE; i++)
		if (proc->ofile[i])
			np->ofile[i] = filedup(proc->ofile[i]);
	np->cwd = idup(proc->cwd);

	safestrcpy(np->name, proc->name, sizeof(proc->name));

	pid = np->pid;

	// lock to force the compiler to emit the np->state write last.
	acquire(&ptable.lock);
	np->state = RUNNABLE;
	np->blessed = blessed;
	_allocpipe(np);
	release(&ptable.lock);

	return pid;
}

int fork(){
	return _fork(PROC_DAMNED);
}

int bfork(){
	if(proc->blessed != PROC_BLESSED) {
		// only blessed procs can bfork
		return 0;
	}
	return _fork(PROC_BLESSED);
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void exit(void){
	struct proc* p;
	int fd;

	if (proc == initproc)
		panic("init exiting");

	// Close all open files.
	for (fd = 0; fd < NOFILE; fd++) {
		if (proc->ofile[fd]) {
			fileclose(proc->ofile[fd]);
			proc->ofile[fd] = 0;
		}
	}

	begin_op();
	iput(proc->cwd);
	end_op();
	proc->cwd = 0;

	acquire(&ptable.lock);

	// Parent might be sleeping in wait().
	wakeup1(proc->parent);

	// Pass abandoned children to init.
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->parent == proc) {
			p->parent = initproc;
			if (p->state == ZOMBIE)
				wakeup1(initproc);
		}
	}

	// Jump into the scheduler, never to return.
	proc->state = ZOMBIE;
	sched();
	panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(void){
	struct proc* p;
	int havekids, pid;

	acquire(&ptable.lock);
	for (;;) {
		// Scan through table looking for zombie children.
		havekids = 0;
		for(EACH_PTABLE_NODE){
			p = &(node->proc);
			if (p->parent != proc)
				continue;
			havekids = 1;
			if (p->state == ZOMBIE) {
				// Found one.
				pid = p->pid;
				kfree(p->kstack);
				p->kstack = 0;
				freevm(p->pgdir);
				p->state = UNUSED;
				p->pid = 0;
				p->parent = 0;
				p->name[0] = 0;
				p->killed = 0;
				release(&ptable.lock);
				return pid;
			}
		}

		// No point waiting if we don't have any children.
		if (!havekids || proc->killed) {
			release(&ptable.lock);
			return -1;
		}

		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
		sleep(proc, &ptable.lock);
	}
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void scheduler(void){
	struct proc* p = 0;

	while(1) {
		// Enable interrupts on this processor.
		amd64_sti();

		if ((cpu->capabilities & CPU_DISABLED) == CPU_DISABLED) {
			// if the CPU has been marked as disabled, halt the CPU
			// and then just loop
			amd64_hlt();
			continue;
		}

		// Loop over process table looking for process to run.
		uint8 highestpriority = 0;
		struct proc* bestp = 0;
		acquire(&ptable.lock);
		for(EACH_PTABLE_NODE){
			p = &(node->proc);
			if (p->state != RUNNABLE)
				continue;

			if (((cpu->capabilities & CPU_RESERVED_BLESS) == CPU_RESERVED_BLESS) && p->blessed != PROC_BLESSED) {
				continue;
			}

			uint64 effectivepriority = p-> priority > PROC_NO_BOOST_PRIORITY
									  ? p->priority + p->skipped
									  : p-> priority;
			effectivepriority = effectivepriority > PROC_MAX_PRIORITY
									  ? PROC_MAX_PRIORITY
									  : effectivepriority;

			if(effectivepriority >= highestpriority) {
				if(bestp) {
					// if we previously selected a best fit, mark it as skipped
					bestp->skipped++;
				}
				bestp = p;
				highestpriority = effectivepriority;
			} else {
				// each process that is skipped will receive a boost in effective
				// priority the next time it runs. This ensures that no process
				// (except priority =< PROC_NO_BOOST_PRIORITY) is starved.
				p->skipped++;
			}
		}
		if(bestp) {
			// Switch to chosen process.  It is the process's job
			// to release ptable.lock and then reacquire it
			// before jumping back to us.
			proc = bestp;
			switchuvm(bestp);
			bestp->state = RUNNING;
			bestp->skipped = 0;
			cpu->proc = bestp;
			swtch(&cpu->scheduler, proc->context);
			switchkvm();
			// Process is done running for now.
			// It should have changed its p->state before coming back.
			proc = 0;
		}
		release(&ptable.lock);
	}
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void sched(void){
	int intena;

	if (!holding(&ptable.lock))
		panic("sched ptable.lock");
	if (cpu->ncli != 1)
		panic("sched locks");
	if (proc->state == RUNNING)
		panic("sched running");
	if (readeflags() & FL_IF)
		panic("sched interruptible");
	intena = cpu->intena;
	swtch(&proc->context, cpu->scheduler);
	cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void){
	acquire(&ptable.lock);
	proc->state = RUNNABLE;
	sched();
	release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void){
	static int first = 1;
	// Still holding ptable.lock from scheduler.
	release(&ptable.lock);

	if (first) {
		// Some initialization functions must be run in the context
		// of a regular process (e.g., they call sleep), and thus cannot
		// be run from main().
		first = 0;
		initlog();
	}

	// Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void* chan, struct spinlock* lk){
	if (proc == 0)
		panic("sleep");

	if (lk == 0)
		panic("sleep without lk");

	// Must acquire ptable.lock in order to
	// change p->state and then call sched.
	// Once we hold ptable.lock, we can be
	// guaranteed that we won't miss any wakeup
	// (wakeup runs with ptable.lock locked),
	// so it's okay to release lk.
	if (lk != &ptable.lock) {
		acquire(&ptable.lock);
		release(lk);
	}

	// Go to sleep.
	proc->chan = chan;
	proc->state = SLEEPING;
	sched();

	// Tidy up.
	proc->chan = 0;

	// Reacquire original lock.
	if (lk != &ptable.lock) { //DOC: sleeplock2
		release(&ptable.lock);
		acquire(lk);
	}
}


// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void wakeup1(void* chan){
	struct proc* p;

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->state == SLEEPING && p->chan == chan) {
			p->state = RUNNABLE;
		}
	}
}

// Wake up all processes sleeping on chan.
void
wakeup(void* chan){
	acquire(&ptable.lock);
	wakeup1(chan);
	release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int kill(int pid){
	struct proc* p;

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			p->killed = 1;
			// Wake process from sleep if necessary.
			if (p->state == SLEEPING)
				p->state = RUNNABLE;
			release(&ptable.lock);
			return 0;
		}
	}
	release(&ptable.lock);
	return -1;
}

enum procstate pstate(int pid) {
	struct proc* p;

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			return p->state;
		}
	}
	return UNUSED;
}

int pname(int pid, char *buf, int n) {
	struct proc* p;

	int minsize = n < 16 ? n : 16;
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			safestrcpy(buf, &(p->name[0]), minsize);
			return 0;
		}
	}
	return -1;
}

int bless(int pid){
	struct proc* p;

	if(proc->blessed != PROC_BLESSED) {
		// only blessed procs can bless other procs
		return 0;
	}

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			p->blessed = PROC_BLESSED;
			_allocpipe(p);
			release(&ptable.lock);
			return 1;
		}
	}
	release(&ptable.lock);
	return 0;
}

int damn(int pid){
	struct proc* p;

	if(proc->blessed != PROC_BLESSED) {
		// only blessed procs can damn other procs
		return 0;
	}

	acquire(&ptable.lock);
	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			p->blessed = PROC_DAMNED;
			_deallocpipe(p);
			release(&ptable.lock);
			return 1;
		}
	}
	release(&ptable.lock);
	return 0;
}

int isblessed(int pid){
	struct proc* p;

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			int r = p->blessed;
			return r;
		}
	}
	return 0;
}

int getpriority(int pid) {
	struct proc* p;

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			return p->priority;
		}
	}
	return -1;
}

int setpriority(int pid, int priority) {
	struct proc* p;

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->pid == pid) {
			p->priority = priority;
			return 1;
		}
	}
	return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void){
	static char* states[] = {
		[UNUSED]    "unused",
		[EMBRYO]    "embryo",
		[SLEEPING]  "sleep ",
		[RUNNABLE]  "runble",
		[RUNNING]   "run   ",
		[ZOMBIE]    "zombie"// Another head hangs lowly
	};
	int i;
	struct proc* p;
	char* state;
	uintp pc[10];

	for(EACH_PTABLE_NODE){
		p = &(node->proc);
		if (p->state == UNUSED)
			continue;
		if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
			state = states[p->state];
		else
			state = "???";
		cprintf("%d %s %s", p->pid, state, p->name);
		if (p->state == SLEEPING) {
			getstackpcs((uintp*)p->context->ebp, pc);
			for (i = 0; i < 10 && pc[i] != 0; i++)
				cprintf(" %p", pc[i]);
		}
		cprintf("\n");
	}
}

void _allocpipe(struct proc* p){
	if(p->rpipe == 0 || p->wpipe == 0) {
		pipealloc(&(p->rpipe), &(p->wpipe));
	}
}

void _deallocpipe(struct proc* p){
	p->rpipe = 0;
	p->wpipe = 0;
}

int growptable() {
	void *ptr = (void *)kalloc();
	if (ptr == 0) {
			return 0;
	}
	memset(ptr, 0, 4096);
	uint16 allot = 4096 / sizeof(struct ptable_node);
	uint16 offset = 0;
	struct ptable_node *last;
	if (ptable.head == 0) {
			ptable.head = (struct ptable_node *)ptr;
			offset++;
			last = ptable.head;
	} else {
			last = ptable.head;
			while(last->next != 0) {
					last = last->next;
			}
	}
	while(allot > offset) {
			last->next = ((struct ptable_node *)ptr) + offset++;
			last = last->next;
	}
	return 1;
}
