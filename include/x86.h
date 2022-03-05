// Routines to let C code use special amd64 instructions.
// See: http://ref.x86asm.net/coder64.html

static inline unsigned char inb(unsigned short port){
	unsigned char data;

	asm volatile ("in %1,%0" : "=a" (data) : "d" (port));
	return data;
}

static inline void amd64_nop(){
	asm volatile ("nop");
}


#define amd64_pause() asm volatile ("pause")

#define amd64_mem_barrier() asm volatile ("lock; addl $0,0(%%rsp)" : : : "memory")

#define amd64_mem_read32(addr) (*(volatile unsigned int *)(addr))
#define amd64_mem_read64(addr) (*(volatile unsigned long *)(addr))

static volatile inline unsigned int amd64_spinread32(unsigned long baseAddr, unsigned int offset){
	short total = 0;
	short same = 0;
	unsigned int val;
	while(total < 1000 && same < 100) {
		amd64_mem_barrier();
		unsigned int newVal = amd64_mem_read32(baseAddr + offset);
		if(val == newVal) {
			same++;
		}else{
			same = 0;
		}
		val = newVal;
		total++;
	}
	return val;
}

static volatile inline unsigned long amd64_spinread64(volatile unsigned long *baseAddr, unsigned int offset){
	short total = 0;
	short same = 0;
	unsigned long val;
	while(total < 1000 && same < 100) {
		amd64_mem_barrier();
		unsigned long newVal = amd64_mem_read32(baseAddr + offset);
		if(val == newVal) {
			same++;
		}else{
			same = 0;
		}
		val = newVal;
		total++;
	}
	return val;
}

#define amd64_cli() cli()

static inline void amd64_insl(int port, void *addr, int cnt){
	asm volatile ("cld; rep insl" :
	              "=D" (addr), "=c" (cnt) :
	              "d" (port), "0" (addr), "1" (cnt) :
	              "memory", "cc");
}

static inline void amd64_out8(unsigned short port, unsigned char data) {
	asm volatile ("out %0,%1" : : "a" (data), "d" (port));
}

static inline unsigned short amd64_in16(unsigned short port){
	unsigned short result;
	asm volatile ( "inw %1, %0"
	               : "=a" (result) : "Nd" (port) );
	return result;
}

static inline void amd64_out16(unsigned short port, unsigned short data) {
	asm volatile ("out %0,%1" : : "a" (data), "d" (port));
}

static inline unsigned int amd64_in32(unsigned short port){
	unsigned int result;
	asm volatile ( "inl %1, %0"
	               : "=a" (result) : "Nd" (port) );
	return result;
}

static inline void amd64_out32(unsigned short port, unsigned int data){
	asm volatile ("outl %0, %1" : : "a" (data), "Nd" (port));
}

static inline unsigned char amd64_in8(unsigned short port){
	unsigned char result;
	asm volatile ( "inb %1, %0"
	               : "=a" (result) : "Nd" (port) );
	return result;
}


static inline void amd64_outsl(int port, const void *addr, int cnt) {
	asm volatile ("cld; rep outsl" :
	              "=S" (addr), "=c" (cnt) :
	              "d" (port), "0" (addr), "1" (cnt) :
	              "cc");
}

static inline void stosb(void *addr, int data, int cnt) {
	asm volatile ("cld; rep stosb" :
	              "=D" (addr), "=c" (cnt) :
	              "0" (addr), "1" (cnt), "a" (data) :
	              "memory", "cc");
}

static inline void stosl(void *addr, int data, int cnt) {
	asm volatile ("cld; rep stosl" :
	              "=D" (addr), "=c" (cnt) :
	              "0" (addr), "1" (cnt), "a" (data) :
	              "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc *p, int size) {
	volatile unsigned short pd[5];

	pd[0] = size-1;
	pd[1] = (unsigned long)p;
	pd[2] = (unsigned long)p >> 16;
#if X64
	pd[3] = (unsigned long)p >> 32;
	pd[4] = (unsigned long)p >> 48;
#endif
	asm volatile ("lgdt (%0)" : : "r" (pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc *p, int size) {
	volatile unsigned short pd[5];

	pd[0] = size-1;
	pd[1] = (unsigned long)p;
	pd[2] = (unsigned long)p >> 16;
#if X64
	pd[3] = (unsigned long)p >> 32;
	pd[4] = (unsigned long)p >> 48;
#endif
	asm volatile ("lidt (%0)" : : "r" (pd));
}

static inline void ltr(unsigned short sel) {
	asm volatile ("ltr %0" : : "r" (sel));
}

static inline unsigned long readeflags(void) {
	unsigned long eflags;
	asm volatile ("pushf; pop %0" : "=r" (eflags));
	return eflags;
}

static inline void loadgs(unsigned short v) {
	asm volatile ("movw %0, %%gs" : : "r" (v));
}

static inline void cli(void) {
	asm volatile ("cli");
}

static inline void amd64_sti(void) {
	asm volatile ("sti");
}

static inline void amd64_hlt(void) {
	asm volatile ("hlt");
}

static inline unsigned int amd64_xchg(volatile unsigned int *addr, unsigned long newval) {
	unsigned int result;

	// The + in "+m" denotes a read-modify-write operand.
	asm volatile ("lock xchgl %0, %1" :
	              "+m" (*addr), "=a" (result) :
	              "1" (newval) :
	              "cc");
	return result;
}

static inline unsigned int amd64_xadd(volatile unsigned int *addr, unsigned long newval) {
	unsigned int result;

	// The + in "+m" denotes a read-modify-write operand.
	asm volatile ("lock xaddl %1, %0" :
	              "+m" (*addr), "=a" (result) :
	              "1" (newval) :
	              "cc");
	return result;
}

static inline unsigned long rcr2(void) {
	unsigned long val;
	asm volatile ("mov %%cr2,%0" : "=r" (val));
	return val;
}

static inline void lcr3(unsigned long val) {
	asm volatile ("mov %0,%%cr3" : : "r" (val));
}

static inline void amd64_cpuid(unsigned int ax, unsigned int *p) {
	asm volatile ("cpuid"
	              : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
	              :  "0" (ax));
}

static inline uint64 amd64_rdmsr(uint msr) {
    uint32 low, high;
    asm volatile ("rdmsr"
		  : "=a" (low), "=d" (high)
		  : "c" (msr));
    return (uint64)low | ((uint64)high << 32);
}

static inline void amd64_wrmsr(uint msr, uint64 newval) {
    uint32 low = newval;
    uint32 high = newval >> 32;
    asm volatile ("wrmsr"
		  :
		  : "a" (low), "d" (high), "c" (msr));
}

static inline uint64 amd64_rdtsc()
{
    uint32 low, high;

    asm volatile("rdtsc" : "=a" (low), "=d" (high));
    return (uint64)low | ((uint64)high << 32);
}


// lie about some register names in 64bit mode to avoid
// clunky ifdefs in proc.c and trap.c.
struct trapframe {
	unsigned long eax; // rax
	unsigned long rbx;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rbp;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long r8;
	unsigned long r9;
	unsigned long r10;
	unsigned long r11;
	unsigned long r12;
	unsigned long r13;
	unsigned long r14;
	unsigned long r15;

	unsigned long trapno;
	unsigned long err;

	unsigned long eip; // rip
	unsigned long cs;
	unsigned long eflags; // rflags
	unsigned long esp; // rsp
	unsigned long ds; // ss
};
