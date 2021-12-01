// Mutual exclusion lock.
struct spinlock {
  uint locked;        // Is the lock held?
  unsigned short sig; // Used by spinlock/holding to differentiate empty ram
                      // from an unacquired lock on CPU#0

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uintp pcs[10];      // The call stack (an array of program counters)
                     // that locked the lock.
};

#define SPINLOCK_SIG 0xAD16
