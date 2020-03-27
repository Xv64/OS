#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "acpi.h"
#include "pci.h"
#include "string.h"

static void bsd4_spam();
static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t* kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int main(void){
    uartearlyinit();
    kinit1(end, P2V(4 * 1024 * 1024)); // phys page allocator
    kvmalloc();    // kernel page table
    if (acpiinit()) // try to use acpi for machine info
        mpinit();  // otherwise use bios MP tables
    if (!ismp)
        panic("too few processors"); //really, it's the year 2020.
    lapicinit();
    seginit();     // set up segments
    cprintf("\ncpu%d: starting Xv64\n\n", cpu->id);
    bsd4_spam();
    picinit();     // interrupt controller
    ioapicinit();  // another interrupt controller
    consoleinit(); // I/O devices & their interrupts
    uartinit();    // serial port
    pinit();       // process table
    tvinit();      // trap vectors
    pciinit();     // initialize PCI bus (AHCI also)
    binit();       // buffer cache
    fileinit();    // file table
    iinit();       // inode cache
    ideinit();     // disk
    // if (!ismp)
    //     timerinit(); // uniprocessor timer
    startothers(); // start other processors
    kinit2(P2V(4 * 1024 * 1024), P2V(PHYSTOP)); // must come after startothers()
    userinit();    // first user process
    // Finish setting up this processor in mpmain.
    mpmain();
}

void bsd4_spam(){
    //I LOVE the BSD license, but the 4-clause license is spammy.
    //put any 4-clause BSD notices in this method
    cprintf("This product includes software developed by Charles M. Hannum, Christopher G. Demetriou\n"); //include/pcireg.hj
}

// Other CPUs jump here from entryother.S.
void mpenter(void){
    switchkvm();
    seginit();
    lapicinit();
    mpmain();
}

// Common CPU setup code.
static void mpmain(void){
    uint32 vendor[4];
    uint32 regs[4];

    amd64_cpuid(0, regs);
    vendor[0] = regs[1];
    vendor[1] = regs[3];
    vendor[2] = regs[2];
    vendor[4] = (uint32)'\0';
    char *cpu_vendor = (char *)vendor;

    idtinit();     // load idt register
    xchg(&cpu->started, 1); // tell startothers() we're up
    cprintf("cpu#%d (%s - %d): ready\n", cpu->id, cpu_vendor, regs[0]);
    if(cpu->id == 0){
        //if this is the primary CPU, display a hello message
        //before we never return again
        cprintf("Welcome to Xv64\n");
    }
    scheduler();   // start running processes
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

void sys_reboot(){
    cprintf("Goodbye\n");
    acpi_reboot();
}
