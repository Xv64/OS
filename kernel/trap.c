#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "irq.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uintp vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

struct spinlock irqHandlersLock;
irqhandler irqHandlers[MAX_IRQS];

irqhandler get_registered_handler(uint16 irq);

void trap(struct trapframe* tf){
    if (tf->trapno == T_SYSCALL) {
        if (proc->killed)
            exit();
        proc->tf = tf;
        syscall();
        if (proc->killed)
            exit();
        return;
    }

    switch (tf->trapno) {
    case T_IRQ0 + IRQ_TIMER:
        if (cpu->id == 0) {
            acquire(&tickslock);
            ticks++;
            wakeup(&ticks);
            release(&tickslock);
        }
        lapiceoi();
        break;
    case T_IRQ0 + IRQ_IDE1:
        ideintr();
        lapiceoi();
        break;
    case T_IRQ0 + IRQ_IDE2:
        ideintr();
        lapiceoi();
        break;
    case T_IRQ0 + IRQ_KBD:
        kbdintr();
        lapiceoi();
        break;
    case T_IRQ0 + IRQ_COM1:
        uartintr();
        lapiceoi();
        break;
    case T_IRQ0 + 7:
    case T_IRQ0 + IRQ_SPURIOUS:
        cprintf("cpu%d: spurious interrupt at %x:%x\n",
                cpu->id, tf->cs, tf->eip);
        lapiceoi();
        break;

    default:
        acquire(&irqHandlersLock);
        void (*dynamicIrqHandler)(uint16) = get_registered_handler(tf->trapno);
        release(&irqHandlersLock);

        if(dynamicIrqHandler){
            //all's good, we found a dyanmic IRQ handler that was defined for this
            dynamicIrqHandler(tf->trapno);
            lapiceoi();
        }else if (proc == 0 || (tf->cs & 3) == 0) {
            // In kernel, it must be our mistake.
            if(tf->trapno == T_PGFLT){
                panic("PAGE FAULT");
            }else{
                cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
                        tf->trapno, cpu->id, tf->eip, rcr2());
                panic("trap");
            }
        } else {
            // In user space, assume process misbehaved.
            cprintf("pid %d %s: trap %d err %d on cpu %d "
                    "eip 0x%x addr 0x%x--kill proc\n",
                    proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip,
                    rcr2());
            proc->killed = 1;
        }
    }

    // Force process exit if it has been killed and is in user space.
    // (If it is still executing in the kernel, let it keep running
    // until it gets to the regular system call return.)
    if (proc && proc->killed && (tf->cs & 3) == DPL_USER)
        exit();

    // Force process to give up CPU on clock tick.
    // If interrupts were on while locks held, would need to check nlock.
    if (proc && proc->state == RUNNING && tf->trapno == T_IRQ0 + IRQ_TIMER)
        yield();

    // Check if the process has been killed since we yielded
    if (proc && proc->killed && (tf->cs & 3) == DPL_USER)
        exit();
}

uint8 irq_register_handler(uint16 irq, irqhandler handler) {
    if(irq >= MAX_IRQS){
        return 0; //sorry
    }
    acquire(&irqHandlersLock);
    irqHandlers[irq] = handler;
    release(&irqHandlersLock);
    return 1;
}

irqhandler get_registered_handler(uint16 irq) {
    irqhandler result = 0;
    if(irq < MAX_IRQS){
        acquire(&irqHandlersLock);
        result = irqHandlers[irq];
        release(&irqHandlersLock);
    }
    return result;
}
