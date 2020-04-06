// Intel 8250 serial port (UART).

#include "types.h"
#include "defs.h"
#include "param.h"
#include "irq.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

#define COM1      0x3F8
#define COM2      0x2F8
#define COM3      0x3E8
#define COM4      0x2E8
#define BAUD_RATE 115200

static int uart;    // is there a uart?

void uartearlyinit(void){
    char* p;

    // Turn off the FIFO
    amd64_out8(COM1 + 2, 0);

    // 115200 baud, 8 data bits, 1 stop bit, parity off.
    amd64_out8(COM1 + 3, 0x80); // Unlock divisor
    amd64_out8(COM1 + 0, 115200 / BAUD_RATE);
    amd64_out8(COM1 + 1, 0);
    amd64_out8(COM1 + 3, 0x03); // Lock divisor, 8 data bits.
    amd64_out8(COM1 + 4, 0);
    amd64_out8(COM1 + 1, 0x01); // Enable receive interrupts.

    // If status is 0xFF, no serial port.
    if (inb(COM1 + 5) == 0xFF)
        return;
    uart = 1;

    // Announce that we're here.
    for (p = "Xv64...\n"; *p; p++)
        uartputc(*p);
}

void uartinit(void){
    if (!uart)
        return;

    // Acknowledge pre-existing interrupt conditions;
    // enable interrupts.
    inb(COM1 + 2);
    inb(COM1 + 0);
    picenable(IRQ_COM1);
    ioapicenable(IRQ_COM1, 0);
}

void uartputc(int c){
    if (!uart){
        return;
    }
    while(!(inb(COM1 + 5) & 0x20)) {
        microdelay(10);
    }
    amd64_out8(COM1 + 0, c);
}

static int uartgetc(void){
    if (!uart)
        return -1;
    if (!(inb(COM1 + 5) & 0x01))
        return -1;
    return inb(COM1 + 0);
}

void uartintr(void){
    consoleintr(uartgetc);
}
