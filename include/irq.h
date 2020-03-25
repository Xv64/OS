#ifndef XV64_IRQ
#define XV64_IRQ

#define T_IRQ0          32      // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER        0
#define IRQ_KBD          1
#define IRQ_COM1         4
#define IRQ_IDE1        14
#define IRQ_IDE2        15
#define IRQ_ERROR       19
#define IRQ_SPURIOUS    31
#define MAX_IRQS        32

typedef void (*irqhandler)(uint16);
uint8 irq_register_handler(uint16 irq, irqhandler handler); //this is defined in trap.c

#endif
