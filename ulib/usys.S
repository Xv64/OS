#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(procexit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(amblessed)
SYSCALL(reboot)
SYSCALL(kconsole_info)
SYSCALL(seek)
SYSCALL(getppid)
SYSCALL(bless)
SYSCALL(damn)
SYSCALL(isblessed)
SYSCALL(bfork)
SYSCALL(mkvdev)
SYSCALL(pstate)
SYSCALL(pname)
SYSCALL(ticks)
SYSCALL(halt)
SYSCALL(info)
SYSCALL(nprocs)
