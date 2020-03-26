#ifndef XV64_NET_H
#define XV64_NET_H

struct packet {
    /*
    If a packet hits a pocket on a socket on a port,
    and the bus is interrupted at a very last resort,
    and the access of the memory makes your floppy disk
    abort, then the socket packet pocket has an error to
    report. - Dr Seuss, probably
    */
    uint16 len;
    uint8 *data;
};

#endif
