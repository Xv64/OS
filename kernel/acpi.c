/* vm64.c
 *
 * Copyright (c) 2020 Jason Whitehorn
 * Copyright (c) 2013 Brian Swetland
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"
#include "acpi.h"
#include "kernel/klib.h"

extern struct cpu cpus[NCPU];
extern int ismp;
extern int ncpu;
extern uchar ioapicid;

//from documentation @ wiki.osdev.org
//https://wiki.osdev.org/Reboot
//START
#define KBRD_INTRFC 0x64
#define KBRD_BIT_KDATA 0
#define KBRD_BIT_UDATA 1
#define KBRD_IO 0x60
#define KBRD_RESET 0xFE
#define bit(n) (1<<(n))
#define check_flag(flags, n) ((flags) & bit(n))
//END

static struct acpi_rdsp* scan_rdsp(uint base, uint len) {
	uchar* p;
	for (p = p2v(base); len >= sizeof(struct acpi_rdsp); len -= 4, p += 4) {
		if (memcmp(p, SIG_RDSP, 8) == 0) {
			uint sum, n;
			for (sum = 0, n = 0; n < 20; n++)
	    #define KBRD_IO 0x60 /* keyboard IO port */
#define KBRD_RESET 0xFE /* reset CPU command */
				sum += p[n];
			if ((sum & 0xff) == 0)
				return (struct acpi_rdsp*)p;
		}
	}
	return (struct acpi_rdsp*)0;
}

void acpi_halt() {
	amd64_out16(0x604, 0x2000); //shutdown (only in QEMU)
	//if issuing a shutdown request failed...
loop:
	amd64_hlt();
	goto loop; //we should never hit this line
}

void acpi_reboot(){
	uint8 temp;

	amd64_cli(); /// disable all IRQs


	//from documentation @ wiki.osdev.org
	//https://wiki.osdev.org/Reboot
	// Clear all keyboard buffers (output and command buffers)
	do{
		temp = inb(KBRD_INTRFC); // empty user data
		if (check_flag(temp, KBRD_BIT_KDATA) != 0)
			inb(KBRD_IO); // empty keyboard data
	} while (check_flag(temp, KBRD_BIT_UDATA) != 0);

	do{
		amd64_out8(KBRD_INTRFC, KBRD_RESET); // pulse CPU reset line
	}while(1); //the above should NEVER fail, but if it does...
}

static struct acpi_rdsp* find_rdsp(void) {
	struct acpi_rdsp* rdsp;
	uintp pa;
	pa = *((ushort*)P2V(0x40E)) << 4; // EBDA
	if (pa && (rdsp = scan_rdsp(pa, 1024)))
		return rdsp;
	return scan_rdsp(0xE0000, 0x20000);
}

static int acpi_config_smp(struct acpi_madt* madt) {
	uint32 lapic_addr;
	uint nioapic = 0;
	uchar* p, * e;

	if (!madt)
		return -1;
	if (madt->header.length < sizeof(struct acpi_madt))
		return -1;

	lapic_addr = madt->lapic_addr_phys;

	p = madt->table;
	e = p + madt->header.length - sizeof(struct acpi_madt);

	while (p < e) {
		uint len;
		if ((e - p) < 2)
			break;
		len = p[1];
		if ((e - p) < len)
			break;
		switch (p[0]) {
		case TYPE_LAPIC: {
			struct madt_lapic* lapic = (void*)p;
			if (len < sizeof(*lapic))
				break;
			if (!(lapic->flags & APIC_LAPIC_ENABLED))
				break;
			cprintf("acpi: cpu#%d apicid %d\n", ncpu, lapic->apic_id);
			cpus[ncpu].id = ncpu;
			cpus[ncpu].apicid = lapic->apic_id;
			ncpu++;
			break;
		}
		case TYPE_IOAPIC: {
			struct madt_ioapic* ioapic = (void*)p;
			if (len < sizeof(*ioapic))
				break;
			cprintf("acpi: ioapic#%d @%x id=%d base=%d\n",
			        nioapic, ioapic->addr, ioapic->id, ioapic->interrupt_base);
			if (nioapic) {
				cprintf("warning: multiple ioapics are not supported");
			} else {
				ioapicid = ioapic->id;
			}
			nioapic++;
			break;
		}
		}
		p += len;
	}

	if (ncpu) {
		ismp = 1;
		lapic = IO2V(((uintp)lapic_addr));
		return 0;
	}

	return -1;
}

#if X64
#define PHYSLIMIT 0x80000000
#else
#define PHYSLIMIT 0x0E000000
#endif

int acpiinit(void) {
	unsigned n, count;
	struct acpi_rdsp* rdsp;
	struct acpi_rsdt* rsdt;
	struct acpi_madt* madt = 0;

	rdsp = find_rdsp();
	if (rdsp->rsdt_addr_phys > PHYSLIMIT)
		goto notmapped;
	rsdt = p2v(rdsp->rsdt_addr_phys);
	count = (rsdt->header.length - sizeof(*rsdt)) / 4;
	for (n = 0; n < count; n++) {
		struct acpi_desc_header* hdr = p2v(rsdt->entry[n]);
		if (rsdt->entry[n] > PHYSLIMIT)
			goto notmapped;
#if DEBUG
		uchar sig[5], id[7], tableid[9], creator[5];
		memmove(sig, hdr->signature, 4); sig[4] = 0;
		memmove(id, hdr->oem_id, 6); id[6] = 0;
		memmove(tableid, hdr->oem_tableid, 8); tableid[8] = 0;
		memmove(creator, hdr->creator_id, 4); creator[4] = 0;
		cprintf("acpi: %s %s %s %x %s %x\n",
		        sig, id, tableid, hdr->oem_revision,
		        creator, hdr->creator_revision);
#endif
		if (!memcmp(hdr->signature, SIG_MADT, 4))
			madt = (void*)hdr;
	}

	return acpi_config_smp(madt);

notmapped:
	cprintf("acpi: tables above 0x%x not mapped.\n", PHYSLIMIT);
	return -1;
}
