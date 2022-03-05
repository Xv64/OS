// Segments in proc->gdt.
#define NSEGS     7
#define PROC_BLESSED 1
#define PROC_DAMNED  0

// Per-CPU state
struct cpu {
  uchar id;                    // index into cpus[] below
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  uint64 capabilities;         // bitmask of capabilities of this CPU

  // Cpu-local storage variables; see below
  void *local;
  struct proc *proc;

  char name[50];               // cpuid name
  char vendor[13];             // cpuid vendor
  uint32 model;                // cpuid model
};

extern struct cpu cpus[NCPU];
extern int ncpu;

#define CPU_RESERVED_BLESS 0x01
#define CPU_DISABLED       0x02

#define PROC_NO_BOOST_PRIORITY 0x0A
#define PROC_DEFAULT_PRIORITY  0x80
#define PROC_MAX_PRIORITY      0xFF

// Per-CPU variables, holding pointers to the
// current cpu and to the current process.
// The asm suffix tells gcc to use "%gs:0" to refer to cpu
// and "%gs:4" to refer to proc.  seginit sets up the
// %gs segment register so that %gs refers to the memory
// holding those two variables in the local cpu's struct cpu.
// This is similar to how thread-local variables are implemented
// in thread libraries such as Linux pthreads.
extern __thread struct cpu *cpu;
extern __thread struct proc *proc;

// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
#if X64
struct context {
  uintp r15;
  uintp r14;
  uintp r13;
  uintp r12;
  uintp r11;
  uintp rbx;
  uintp ebp; //rbp
  uintp eip; //rip;
};
#else
struct context {
  uintp edi;
  uintp esi;
  uintp ebx;
  uintp ebp;
  uintp eip;
};
#endif

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  uintp sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  int lastsyscall;
  uint8 blessed;
  uint8 priority;
  uint32 skipped;

  // rpipe & wpipe are only used by blessed processes
  // both are named from the perspective of the kernel
  struct file *rpipe;   // read
  struct file *wpipe;   // write
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
