#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "acpi.h"
#include "pci.h"
#include "buf.h"
#include "irq.h"
#include "kernel/string.h"

static void identcpu();
static void credits();
static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t* kpgdir;
uint64 ROOT_DEV = 1;

extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int main(void){
	uartearlyinit();
	kinit1(P2V(KALLOC_START), P2V(KALLOC_START + 4 * 1024 * 1024)); // phys page allocator
	kvmalloc(); // kernel page table
	trapinit();
	if (acpiinit()) // try to use acpi for machine info
		mpinit(); // otherwise use bios MP tables
	if (!ismp)
		panic("too few processors"); //really, it's the year 2021.

	seginit(); // set up segments
	tvinit();  // trap vectors
	picinit(); // interrupt controller
	ioapicinit(); // another interrupt controller
	uartinit(); // serial port
	pinit();   // process table
	identcpu();
	lapicinit();
	startothers(); // start other processors

	// Finish setting up this processor in mpmain.
	mpmain();
}

void postinit()
{
	consoleinit(); // I/O devices & their interrupts
	cprintf("\ncpu%d: starting Xv64\n\n", cpu->id);
	credits();
	procloopinit();// setup proc loop device
	pciinit(); // initialize PCI bus (AHCI also)
	binit();   // buffer cache
	fileinit(); // file table
	ideinit(); // init IDE disks

	cprintf("Root dev: disk(%d, %d)\n", GETDEVTYPE(ROOT_DEV), GETDEVNUM(ROOT_DEV));
	vfsinit();   // bootstrap fs init
	             // (must happen after all disk types are initialized)

	userinit(); // first user process
}

void credits(){
	cprintf("Xv64 is Copyright (c) 1997-2021, contributors.\nSee LICENSE for details.\n");
}

// Other CPUs jump here from entryother.S.
void mpenter(void){
	switchkvm();
	seginit();
	identcpu();
	lapicinit();
	mpmain();
}

// Common CPU setup code.
static void mpmain(void){
	idtinit(); // load idt register
	if(cpu->id == 0) {
		kinit2(P2V(KALLOC_START + 4 * 1024 * 1024), P2V(PHYSTOP)); // must come after startothers()
		cpu->capabilities = CPU_RESERVED_BLESS;
		cprintf("%d-way SMP kernel online.\n", ncpu);
		postinit();
		cprintf("starting scheduler\n");
	}
	amd64_xchg(&cpu->started, 1); // tell startothers() we're up
	scheduler(); // start running processes
}

void entry32mp(void);

// Start the non-boot (AP) processors.
static void startothers(void){
	extern uchar _binary_out_entryother_start[], _binary_out_entryother_size[];
	uchar* code;
	struct cpu* c;
	char* stack;

	// Write entry code to unused memory at 0x7000.
	// The linker has placed the image of entryother.S in
	// _binary_entryother_start.
	code = p2v(0x7000);
	memmove(code, _binary_out_entryother_start, (uintp)_binary_out_entryother_size);

	for (c = cpus; c < cpus + ncpu; c++) {
		if (c == cpus + cpunum()) // We've started already.
			continue;

		// Tell entryother.S what stack to use, where to enter, and what
		// pgdir to use. We cannot use kpgdir yet, because the AP processor
		// is running in low  memory, so we use entrypgdir for the APs too.
		stack = kalloc();
		*(uint32*)(code - 4) = 0x8000; // just enough stack to get us to entry64mp
		*(uint32*)(code - 8) = v2p(entry32mp);
		*(uint64*)(code - 16) = (uint64)(stack + KSTACKSIZE);

		lapicstartap(c->apicid, v2p(code));

		// wait for cpu to finish mpmain()
		while (c->started == 0)
			;
	}
}

uint32 tscfreq;

enum { eax, ebx, ecx, edx };

void identcpu() {
	uint32 cpuid_0[4];
	uint32 cpuid_8_2[4];
	uint32 cpuid_8_3[4];
	uint32 cpuid_8_4[4];
	uint32 cpuid_16[4];
	uint32 cpuid_vmw[4];

	amd64_cpuid(0, cpuid_0);
	amd64_cpuid(0x80000002, cpuid_8_2);
	amd64_cpuid(0x80000003, cpuid_8_3);
	amd64_cpuid(0x80000004, cpuid_8_4);

	memset(cpu->vendor, 0, sizeof(cpu->vendor));
	memmove(cpu->vendor + 0, &cpuid_0[1], 4);
	memmove(cpu->vendor + 4, &cpuid_0[3], 4);
	memmove(cpu->vendor + 8, &cpuid_0[2], 4);
	cpu->model = cpuid_0[0];

	memset(cpu->name, 0, sizeof(cpu->name));
	memmove(cpu->name + 0x00, cpuid_8_2, 0x10);
	memmove(cpu->name + 0x10, cpuid_8_3, 0x10);
	memmove(cpu->name + 0x20, cpuid_8_4, 0x10);

	amd64_cpuid(0x40000010, cpuid_vmw);
	amd64_cpuid(0x16, cpuid_16);

	if (cpuid_16[eax]) {
		tscfreq = cpuid_16[eax] & 0xffff;
	} else if (cpuid_vmw[eax] > 100000) {
		tscfreq = cpuid_vmw[eax] / 1000;
	}

	cprintf("cpu-%d: name: %s vendor: %s model: %d tscfreq: %d\n",
		cpu->id, cpu->name, cpu->vendor, cpu->model, tscfreq);
}

void sys_reboot(){
	cprintf("Goodbye\n");
	acpi_reboot();
}

void sys_halt(){
	cprintf("Xv64 is shutting down now. Goodbye...\n");
	halt();
}

int sys_info(void) {
	char *buf;
	int n;

	if (argint(1, &n) < 0 || argptr(0, &buf, n) < 0)
		return -1;

	char str[] = "Xv64 0.25 xx-way SMP kernel amd64";
	int tens = ncpu / 10;
	int ones = ncpu - (tens * 10);

	str[10] = 48 + tens;
	str[11] = 48 + ones;

	memcopy(buf, str, sizeof(str));
	return sizeof(str);
}

int sys_nprocs() {
	return ncpu;
}
