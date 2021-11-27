// Copyright (c) 2012-2020 YAMAMOTO Masaya
// SPDX-License-Identifier: MIT

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "pci.h"
#include "proc.h"
#include "net.h"
#include "assert.h"
#include "kernel/klib.h"
#include "e1000_dev.h"
#include "pci.h"

#define RX_RING_SIZE 16
#define TX_RING_SIZE 16

struct e1000 {
    uint32 mmio_base;
    struct rx_desc rx_ring[RX_RING_SIZE] __attribute__((aligned(16)));;
    struct tx_desc tx_ring[TX_RING_SIZE] __attribute__((aligned(16)));;
    uint8 addr[6];
    uint8 irq;
    struct netdev *netdev;
    struct e1000 *next;
};

static struct e1000 *devices;

uint32 e1000_reg_read(struct e1000 *dev, uint16 reg) {
    volatile uint32 *ptr = (volatile uint32 *)(uintptr_t)dev->mmio_base;
    ptr += reg;
    return *ptr;
}

void e1000_reg_write(struct e1000 *dev, uint16 reg, uint32 val) {
    *(volatile uint32 *)((uintptr_t)(dev->mmio_base + reg)) = val;
}

static uint16 e1000_eeprom_read(struct e1000 *dev, uint8 addr) {
    uint32 eerd;
    e1000_reg_write(dev, E1000_EERD, E1000_EERD_READ | addr << E1000_EERD_ADDR);
    while (!((eerd = e1000_reg_read(dev, E1000_EERD)) & E1000_EERD_DONE))
        microdelay(1);
    return (uint16)(eerd >> E1000_EERD_DATA);
}

static void e1000_read_addr_from_eeprom(struct e1000 *dev, uint8 *dst) {
    uint16 data;
    for (int n = 0; n < 3; n++) {
        data = e1000_eeprom_read(dev, n);
        dst[n*2+0] = (data & 0xff);
        dst[n*2+1] = (data >> 8) & 0xff;
    }
}

static uint32 e1000_resolve_mmio_base(struct pci_func *pcif) {
    uint32 mmio_base = 0;
    for (int n = 0; n < 6; n++) {
        if (pcif->reg_base[n] > 0xffff) {
            assert(pcif->reg_size[n] == (1<<17));
            mmio_base = pcif->reg_base[n];
            break;
        }
    }
    return mmio_base;
}

static void e1000_rx_init(struct e1000 *dev) {
    // initialize rx descriptors
    for(int n = 0; n < RX_RING_SIZE; n++) {
        memset(&dev->rx_ring[n], 0, sizeof(struct rx_desc));
        // alloc DMA buffer
        dev->rx_ring[n].addr = (uint64)V2P(kalloc());
    }
    // setup rx descriptors
    uint64 base = V2P(dev->rx_ring);
    e1000_reg_write(dev, E1000_RDBAL, (uint32)(base & 0xffffffff));
    e1000_reg_write(dev, E1000_RDBAH, (uint32)(base >> 32));
    // rx descriptor lengh
    e1000_reg_write(dev, E1000_RDLEN, (uint32)(RX_RING_SIZE * sizeof(struct rx_desc)));
    // setup head/tail
    e1000_reg_write(dev, E1000_RDH, 0);
    e1000_reg_write(dev, E1000_RDT, RX_RING_SIZE-1);
    // set tx control register
    e1000_reg_write(dev, E1000_RCTL, (
        E1000_RCTL_SBP        | /* store bad packet */
        E1000_RCTL_UPE        | /* unicast promiscuous enable */
        E1000_RCTL_MPE        | /* multicast promiscuous enab */
        E1000_RCTL_RDMTS_HALF | /* rx desc min threshold size */
        E1000_RCTL_SECRC      | /* Strip Ethernet CRC */
        E1000_RCTL_LPE        | /* long packet enable */
        E1000_RCTL_BAM        | /* broadcast enable */
        E1000_RCTL_SZ_2048    | /* rx buffer size 2048 */
        0)
    );
}

static void e1000_tx_init(struct e1000 *dev) {
    // initialize tx descriptors
    for (int n = 0; n < TX_RING_SIZE; n++) {
        memset(&dev->tx_ring[n], 0, sizeof(struct tx_desc));
    }
    // setup tx descriptors
    uint64 base = (uint64)(V2P(dev->tx_ring));
    e1000_reg_write(dev, E1000_TDBAL, (uint32)(base & 0xffffffff));
    e1000_reg_write(dev, E1000_TDBAH, (uint32)(base >> 32) );
    // tx descriptor length
    e1000_reg_write(dev, E1000_TDLEN, (uint32)(TX_RING_SIZE * sizeof(struct tx_desc)));
    // setup head/tail
    e1000_reg_write(dev, E1000_TDH, 0);
    e1000_reg_write(dev, E1000_TDT, 0);
    // set tx control register
    e1000_reg_write(dev, E1000_TCTL, (
        E1000_TCTL_PSP | /* pad short packets */
        0)
    );
}

static int e1000_open(struct netdev *netdev) {
    struct e1000 *dev = (struct e1000 *)netdev->priv;
    // enable interrupts
    e1000_reg_write(dev, E1000_IMS, E1000_IMS_RXT0);
    // clear existing pending interrupts
    e1000_reg_read(dev, E1000_ICR);
    // enable RX/TX
    e1000_reg_write(dev, E1000_RCTL, e1000_reg_read(dev, E1000_RCTL) | E1000_RCTL_EN);
    e1000_reg_write(dev, E1000_TCTL, e1000_reg_read(dev, E1000_TCTL) | E1000_TCTL_EN);
    // link up
    e1000_reg_write(dev, E1000_CTL, e1000_reg_read(dev, E1000_CTL) | E1000_CTL_SLU);
    netdev->flags |= NETDEV_FLAG_UP;
    return 0;
}

static int e1000_stop(struct netdev *netdev) {
    struct e1000 *dev = (struct e1000 *)netdev->priv;
    // disable interrupts
    e1000_reg_write(dev, E1000_IMC, E1000_IMS_RXT0);
    // clear existing pending interrupts
    e1000_reg_read(dev, E1000_ICR);
    // disable RX/TX
    e1000_reg_write(dev, E1000_RCTL, e1000_reg_read(dev, E1000_RCTL) & ~E1000_RCTL_EN);
    e1000_reg_write(dev, E1000_TCTL, e1000_reg_read(dev, E1000_TCTL) & ~E1000_TCTL_EN);
    // link down
    e1000_reg_write(dev, E1000_CTL, e1000_reg_read(dev, E1000_CTL) & ~E1000_CTL_SLU);
    netdev->flags &= ~NETDEV_FLAG_UP;
    return 0;
}

static int32 e1000_tx_cb(struct netdev *netdev, uint8 *data, uint32 len) {
    struct e1000 *dev = (struct e1000 *)netdev->priv;
    uint32 tail = e1000_reg_read(dev, E1000_TDT);
    struct tx_desc *desc = &dev->tx_ring[tail];

    desc->addr = (uint64)V2P(data);
    desc->length = len;
    desc->status = 0;
    desc->cmd = (E1000_TXD_CMD_EOP | E1000_TXD_CMD_RS);
#ifdef DEBUG
    cprintf("[e1000] %s: %u bytes data transmit\n", dev->netdev->name, desc->length);
#endif
    e1000_reg_write(dev, E1000_TDT, (tail + 1) % TX_RING_SIZE);
    while(!(desc->status & 0x0f)) {
        microdelay(1);
    }
    return len;
}

static int32 e1000_tx(struct netdev *dev, uint16 type, const uint8 *packet, uint32 len, const void *dst) {
    return ethernet_tx_helper(dev, type, packet, len, dst, e1000_tx_cb);
}

static void e1000_rx(struct e1000 *dev) {
#ifdef DEBUG
    cprintf("[e1000] %s: check rx descriptors...\n", dev->netdev->name);
#endif
    while (1) {
        uint32 tail = (e1000_reg_read(dev, E1000_RDT)+1) % RX_RING_SIZE;
        struct rx_desc *desc = &dev->rx_ring[tail];
        if (!(desc->status & E1000_RXD_STAT_DD)) {
            /* EMPTY */
            break;
        }
        do {
            if (desc->length < 60) {
                cprintf("[e1000] short packet (%d bytes)\n", desc->length);
                break;
            }
            if (!(desc->status & E1000_RXD_STAT_EOP)) {
                cprintf("[e1000] not EOP! this driver does not support packet that do not fit in one buffer\n");
                break;
            }
            if (desc->errors) {
                cprintf("[e1000] rx errors (0x%x)\n", desc->errors);
                break;
            }
#ifdef DEBUG
            cprintf("[e1000] %s: %u bytes data received\n", dev->netdev->name, desc->length);
#endif
            ethernet_rx_helper(dev->netdev, P2V(desc->addr), desc->length, netdev_receive);
        } while (0);
        desc->status = (uint16)(0);
        e1000_reg_write(dev, E1000_RDT, tail);
    }
}

void e1000intr(void) {
    struct e1000 *dev;
    int icr;
#ifdef DEBUG
    cprintf("[e1000] interrupt: etner\n");
#endif
    for (dev = devices; dev; dev = dev->next) {
        icr = e1000_reg_read(dev, E1000_ICR);
        if (icr & E1000_ICR_RXT0) {
            e1000_rx(dev);
            // clear pending interrupts
            e1000_reg_read(dev, E1000_ICR);
        }
    }
#ifdef DEBUG
    cprintf("[e1000] interrupt: leave\n");
#endif
}

void e1000_setup(struct netdev *dev) {
    ethernet_netdev_setup(dev);
}

struct netdev_ops e1000_ops = {
    .open = e1000_open,
    .stop = e1000_stop,
    .xmit = e1000_tx,
};

int32 e1000_init(struct pci_func *pcif) {
    pci_func_enable(pcif);
    struct e1000 *dev = (struct e1000 *)kalloc();
    // Resolve MMIO base address
    dev->mmio_base = e1000_resolve_mmio_base(pcif);
    assert(dev->mmio_base);
    cprintf("[e1000] mmio_base=0x%08x\n", dev->mmio_base);
    // Read HW address from EEPROM
    e1000_read_addr_from_eeprom(dev, dev->addr);
    cprintf("[e1000] addr=%02x:%02x:%02x:%02x:%02x:%02x\n", dev->addr[0], dev->addr[1], dev->addr[2], dev->addr[3], dev->addr[4], dev->addr[5]);
    // Register I/O APIC
    dev->irq = pcif->irq_line;
    ioapicenable(dev->irq, ncpu - 1);
    // Initialize Multicast Table Array
    for (int n = 0; n < 128; n++)
        e1000_reg_write(dev, E1000_MTA + (n << 2), 0);
    // Initialize RX/TX
    e1000_rx_init(dev);
    e1000_tx_init(dev);
    // Alloc netdev
    struct netdev *netdev = netdev_alloc(e1000_setup);
    memcpy(netdev->addr, dev->addr, 6);
    netdev->priv = dev;
    netdev->ops = &e1000_ops;
    netdev->flags |= NETDEV_FLAG_RUNNING;
    // Register netdev
    netdev_register(netdev);
    dev->netdev = netdev;
    // Link to e1000 device list
    dev->next = devices;
    devices = dev;
    return 0;
}
