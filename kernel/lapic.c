// The local APIC manages internal (non-I/O) interrupts.

#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "irq.h"
#include "mmu.h"
#include "x86.h"
#include "param.h"
#include "proc.h"

// Local APIC registers, divided by 4 for use as uint[] indices.
#define ID               0x02   // ID
#define VER              0x03   // Version
#define TPR              0x08   // Task Priority
#define EOI              0x0B   // EOI
#define LDR              0x0D   // Logical destination
#define SVR              0x0F   // Spurious Interrupt Vector
#define ENABLE     0x00000100   // Unit Enable
#define ISR0             0x10   // In-service bits 0:31
#define ISR7             0x17   // In-service bits 224:255
#define TMR0             0x18   // Trigger mode bits 0:31
#define TMR7             0x1F   // Trigger mode bits 224:255
#define IRR0             0x20   // Interrupt request bits 0:31
#define IRR7             0x27   // Interrupt request bits 224:255
#define ESR              0x28   // Error Status
#define ICRLO            0x30   // Interrupt Command [31:0]
#define ICRHI            0x31   // Interrupt Command [63:32]
#define INIT       0x00000500   // INIT/RESET
#define STARTUP    0x00000600   // Startup IPI
#define DELIVS     0x00001000   // Delivery status
#define ASSERT     0x00004000   // Assert interrupt (vs deassert)
#define DEASSERT   0x00000000
#define LEVEL      0x00008000   // Level triggered
#define BCAST      0x00080000   // Send to all APICs, including self.
#define BUSY       0x00001000
#define FIXED      0x00000000
#define TIMER            0x32   // Local Vector Table 0 (TIMER)
#define X1         0x0000000B   // divide counts by 1
#define PERIODIC   0x00020000   // Periodic
#define THERMAL          0x33   // Thermal Monitoring LVT
#define PCINT            0x34   // Performance Counter LVT
#define LINT0            0x35   // Local Vector Table 1 (LINT0)
#define LINT1            0x36   // Local Vector Table 2 (LINT1)
#define ERROR            0x37   // Local Vector Table 3 (ERROR)
#define MASKED     0x00010000   // Interrupt masked
#define TICR             0x38   // Timer Initial Count
#define TCCR             0x39   // Timer Current Count
#define TDCR             0x3E   // Timer Divide Configuration

volatile uint* lapic;  // Initialized in mp.c
const int wants_x2apic = 1;
__thread volatile int using_x2apic = 0;

static inline int has_x2apic() {
	uint32 cpuid_1[4];
	enum { eax, ebx, ecx, edx };
	amd64_cpuid(1, cpuid_1);
	return !!(cpuid_1[ecx] & CPUID_X2APIC);
}

static inline void lapicw(int index, int value) {
	if (using_x2apic) {
		amd64_wrmsr(0x800+index, value);
	} else {
		lapic[index*4] = value;
		lapic[ID*4]; // wait for write to finish, by reading
	}
}

static inline uint64 lapicr(int index) {
	if (using_x2apic) {
		return amd64_rdmsr(0x800+index);
	} else {
		return lapic[index*4];
	}
}

static inline void lapic_wicr(uint32 id, uint32 value)
{
	if (using_x2apic) {
		amd64_wrmsr(0x800+ICRLO, ((uint64)id << 32) | (uint64)value);
	} else {
		lapicw(ICRHI, (int32)id);
		lapicw(ICRLO, (int32)value);
	}
}

static inline uint64 lapic_ricr()
{
	if (using_x2apic) {
		return amd64_rdmsr(0x800+ICRLO);
	} else {
		return ((uint64)lapicr(ICRHI) << 32) | (uint64)lapicr(ICRLO);
	}
}

void lapicinit(void){
	if (!lapic)
		return;

	// Enable x2apic mode if available
	lapicx2enable(wants_x2apic);

	// Read and report APIC base, id, and version fields
	uint64 apicbase = amd64_rdmsr(MSR_IA32_APIC_BASE);
	uint64 apicaddr = apicbase & ~((1ul << 12) - 1);
	uint xapic = !!(apicbase & (1ul << 11));
	uint x2apic = !!(apicbase & (1ul << 10));
	uint bsp = !!(apicbase & (1ul << 8));
	uint apicid = lapicr(ID) >> (using_x2apic ? 0 : 24);
	uint apicver = lapicr(VER);
	uint version = apicver & 0xff;
	uint maxlvt = (apicver >> 16) & 0xFF;

	cprintf("cpu-%d: lapicinit: lapic#%x @%x, version=0x%x, maxlvt=%d, xapic=%d, x2apic=%d, bsp=%d\n",
		cpu->id, apicid, apicaddr, version, maxlvt, xapic, x2apic, bsp);

	// Enable local APIC; set spurious interrupt vector.
	lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));

	// The timer repeatedly counts down at bus frequency
	// from lapic[TICR] and then issues an interrupt.
	// If xv6 cared more about precise timekeeping,
	// TICR would be calibrated using an external time source.
	lapicw(TDCR, X1);
	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);

	// Disable logical interrupt lines.
	lapicw(LINT0, MASKED);
	lapicw(LINT1, MASKED);

	// Disable performance counter and thermal interrupts
	// on machines that provide those LVT entries.
	if (maxlvt >= 5) {
		lapicw(PCINT, MASKED);
		lapicw(THERMAL, MASKED);
	}

	// Map error interrupt to IRQ_ERROR.
	lapicw(ERROR, T_IRQ0 + IRQ_ERROR);

	// Clear error status register (requires back-to-back writes).
	lapicw(ESR, 0);
	lapicw(ESR, 0);

	// Ack any outstanding interrupts.
	if (using_x2apic) {
		for (uint isr_reg = ISR7; isr_reg >= ISR0; isr_reg--) {
			for (uint i = 0U; i < 32U; i++) {
				if (lapicr(isr_reg) != 0U) {
					lapicw(EOI, 0U);
				} else {
					break;
				}
			}
		}
	} else {
		lapicw(EOI, 0);
	}

	// Send an Init Level De-Assert to synchronise arbitration ID's.
	lapic_wicr(0, BCAST | INIT | LEVEL);
	while (lapic_ricr() & DELIVS)
		;

	// Enable interrupts on the APIC (but not on the processor).
	lapicw(TPR, 0);
}

// This is only used during secondary processor startup.
// cpu->id is the fast way to get the cpu number, once the
// processor is fully started.
int cpunum(void){
	int n, id;
	// Cannot call cpu when interrupts are enabled:
	// result not guaranteed to last long enough to be used!
	// Would prefer to panic but even printing is chancy here:
	// almost everything, including cprintf and panic, calls cpu,
	// often indirectly through acquire and release.
	if (readeflags() & FL_IF) {
		static int n;
		if (n++ == 0)
			cprintf("cpu called from %x with interrupts enabled\n",
			        __builtin_return_address(0));
	}

	if (!lapic)
		return 0;

	id = lapicr(ID) >> (using_x2apic ? 0 : 24);
	for (n = 0; n < ncpu; n++)
		if (id == cpus[n].apicid)
			return n;

	return 0;
}

// Acknowledge interrupt.
void lapiceoi(void){
	if (lapic)
		lapicw(EOI, 0);
}

extern uint32 tscfreq;

// Spin for a given number of microseconds using the tsc
void microdelay(int us){
	uint64 tc, ts = amd64_rdtsc(), tf = ts + (uint64)tscfreq * (uint64)us;
	while ((tc = amd64_rdtsc()) < tf)
		;
}

#define IO_RTC  0x70

// Start additional processor running entry code at addr.
// See Appendix B of MultiProcessor Specification.
void lapicstartap(uchar apicid, uint addr){
	int i;
	ushort* wrv;

	// "The BSP must initialize CMOS shutdown code to 0AH
	// and the warm reset vector (DWORD based at 40:67) to point at
	// the AP startup code prior to the [universal startup algorithm]."
	amd64_out8(IO_RTC, 0xF); // offset 0xF is shutdown code
	amd64_out8(IO_RTC + 1, 0x0A);
	wrv = (ushort*)P2V((0x40 << 4 | 0x67)); // Warm reset vector
	wrv[0] = 0;
	wrv[1] = addr >> 4;

	uint destid = apicid << (using_x2apic ? 0 : 24);

	// "Universal startup algorithm."
	// Send INIT (level-triggered) interrupt to reset other CPU.
	lapic_wicr(destid, INIT | LEVEL | ASSERT);
	microdelay(200);
	lapic_wicr(destid, INIT | LEVEL);
	microdelay(100); // should be 10ms, but too slow in Bochs!

	// Send startup IPI (twice!) to enter code.
	// Regular hardware is supposed to only accept a STARTUP
	// when it is in the halted state due to an INIT.  So the second
	// should be ignored, but it is part of the official Intel algorithm.
	// Bochs complains about the second one.  Too bad for Bochs.
	for (i = 0; i < 2; i++) {
		lapic_wicr(destid, STARTUP | (addr >> 12));
		microdelay(200);
	}
}

// enable x2apic mode
void lapicx2enable(uint enable)
{
	enum { X2APIC_ENABLE = (1ul << 10) };

	uint64 apicbase = amd64_rdmsr(MSR_IA32_APIC_BASE);

	if (!has_x2apic()) return;

	if (enable && (apicbase & X2APIC_ENABLE) == 0) {
		amd64_wrmsr(MSR_IA32_APIC_BASE, (apicbase |= X2APIC_ENABLE));
	} else if (!enable && (apicbase & X2APIC_ENABLE) == X2APIC_ENABLE) {
		amd64_wrmsr(MSR_IA32_APIC_BASE, (apicbase &= ~X2APIC_ENABLE));
	}
	using_x2apic = !!enable;
}

// send ipi to another processor
void lapicsendipi(uint irq, uint cpunum)
{
	lapic_wicr(using_x2apic ? cpunum : cpunum << 24, irq);
}
