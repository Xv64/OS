// Copyright (c) 2012-2020 YAMAMOTO Masaya
// SPDX-License-Identifier: MIT

#include "types.h"
#include "defs.h"
#include "net.h"
#include "ip.h"
#include "kernel/klib.h"

struct netproto {
    struct netproto *next;
    uint16 type;
    void (*handler)(uint8 *packet, uint32 plen, struct netdev *dev);
};

static struct netdev *devices;
static struct netproto *protocols;

struct netdev *netdev_root(void){
    return devices;
}

struct netdev *netdev_alloc(void (*setup)(struct netdev *)){
    struct netdev *dev;
    static unsigned int index = 0;

    dev = (struct netdev *)kalloc();
    if (!dev) {
        return 0x0;
    }
    memset(dev, 0, sizeof(struct netdev));
    dev->index = index++;
    snprintf(dev->name, sizeof(dev->name), "net%d", dev->index);
    setup(dev);
    return dev;
}

int netdev_register(struct netdev *dev){
    cprintf("[net] netdev_register: <%s>\n", dev->name);
    dev->next = devices;
    devices = dev;
    return 0;
}

struct netdev *netdev_by_index(int index){
    struct netdev *dev;

    for (dev = devices; dev; dev = dev->next)
        if (dev->index == index)
            return dev;

    return 0x0;
}

struct netdev *netdev_by_name(const char *name){
    struct netdev *dev;

    for (dev = devices; dev; dev = dev->next)
        if (strcmp(dev->name, name) == 0)
            return dev;

    return 0x0;
}

void netdev_receive(struct netdev *dev, uint16 type, uint8 *packet, unsigned int plen) {
    struct netproto *entry;
#ifdef DEBUG
    cprintf("[net] netdev_receive: dev=%s, type=%04x, packet=%p, plen=%u\n", dev->name, type, packet, plen);
#endif
    for (entry = protocols; entry; entry = entry->next) {
        if (hton16(entry->type) == type) {
            entry->handler(packet, plen, dev);
            return;
        }
    }
}

int netdev_add_netif(struct netdev *dev, struct netif *netif){
    struct netif *entry;

    for (entry = dev->ifs; entry; entry = entry->next) {
        if (entry->family == netif->family) {
            return -1;
        }
    }
#ifdef DEBUG
    if (netif->family == NETIF_FAMILY_IPV4) {
        char addr[IP_ADDR_STR_LEN];
        cprintf("[net] Add <%s> to <%s>\n", ip_addr_ntop(&((struct netif_ip *)netif)->unicast, addr, sizeof(addr)), dev->name);
    }
#endif
    netif->next = dev->ifs;
    netif->dev  = dev;
    dev->ifs = netif;
    return 0;
}

struct netif *netdev_get_netif(struct netdev *dev, int family){
    struct netif *entry;

    for (entry = dev->ifs; entry; entry = entry->next) {
        if (entry->family == family) {
            return entry;
        }
    }
    return 0x0;
}

int netproto_register(unsigned short type, void (*handler)(uint8 *packet, uint32 plen, struct netdev *dev)){
    struct netproto *entry;

    for (entry = protocols; entry; entry = entry->next) {
        if (entry->type == type) {
            return -1;
        }
    }
    entry = (struct netproto *)kalloc();
    if (!entry) {
        return -1;
    }
    entry->next = protocols;
    entry->type = type;
    entry->handler = handler;
    protocols = entry;
    return 0;
}

void netinit(void) {
    arp_init();
    ip_init();
    icmp_init();
    udp_init();
    tcp_init();
}
