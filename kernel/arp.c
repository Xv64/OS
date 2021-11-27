// Copyright (c) 2012-2020 YAMAMOTO Masaya
// SPDX-License-Identifier: MIT

#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "net.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"
#include "kernel/klib.h"

#define ARP_HRD_ETHERNET 0x0001

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2

#define ARP_TABLE_SIZE 4096
#define ARP_TABLE_TIMEOUT_SEC 300

#define array_tailof(x) (x + (sizeof(x) / sizeof(*x)))

struct arp_hdr {
    uint16 hrd;
    uint16 pro;
    uint8 hln;
    uint8 pln;
    uint16 op;
};

struct arp_ethernet {
    struct arp_hdr hdr;
    uint8 sha[ETHERNET_ADDR_LEN];
    ip_addr_t spa;
    uint8 tha[ETHERNET_ADDR_LEN];
    ip_addr_t tpa;
} __attribute__ ((packed));

struct arp_entry {
    unsigned char used;
    ip_addr_t pa;
    uint8 ha[ETHERNET_ADDR_LEN];
    time_t timestamp;
    void *data;
    uint32 len;
    struct netif *netif;
};

static struct spinlock arplock;
static struct arp_entry arp_table[ARP_TABLE_SIZE];
static time_t timestamp;

static char *arp_opcode_ntop (uint16 opcode) {
    switch (ntoh16(opcode)) {
    case ARP_OP_REQUEST:
        return "REQUEST";
    case ARP_OP_REPLY:
        return "REPLY";
    }
    return "UNKNOWN";
}

void arp_dump (uint8 *packet, uint32 plen) {
    struct arp_ethernet *message;
    char addr[128];

    message = (struct arp_ethernet *)packet;
    cprintf(" hrd: 0x%04x\n", ntoh16(message->hdr.hrd));
    cprintf(" pro: 0x%04x\n", ntoh16(message->hdr.pro));
    cprintf(" hln: %u\n", message->hdr.hln);
    cprintf(" pln: %u\n", message->hdr.pln);
    cprintf("  op: %u (%s)\n", ntoh16(message->hdr.op), arp_opcode_ntop(message->hdr.op));
    cprintf(" sha: %s\n", ethernet_addr_ntop(message->sha, addr, sizeof(addr)));
    cprintf(" spa: %s\n", ip_addr_ntop(&message->spa, addr, sizeof(addr)));
    cprintf(" tha: %s\n", ethernet_addr_ntop(message->tha, addr, sizeof(addr)));
    cprintf(" tpa: %s\n", ip_addr_ntop(&message->tpa, addr, sizeof(addr)));
}

static struct arp_entry *arp_table_select (const ip_addr_t *pa) {
    struct arp_entry *entry;

    for (entry = arp_table; entry < array_tailof(arp_table); entry++) {
        if (entry->used && entry->pa == *pa) {
            return entry;
        }
    }
    return 0x0;
}

static int arp_table_update (struct netdev *dev, const ip_addr_t *pa, const uint8 *ha) {
    struct arp_entry *entry;

    entry = arp_table_select(pa);
    if (!entry) {
        return -1;
    }
    memcpy(entry->ha, ha, ETHERNET_ADDR_LEN);
    time(&entry->timestamp);
    if (entry->data) {
        if (entry->netif->dev != dev) {
            /* warning: receive response from unintended device */
            dev = entry->netif->dev;
        }
        dev->ops->xmit(dev, ETHERNET_TYPE_IP, (uint8 *)entry->data, entry->len, entry->ha);
        kfree(entry->data);
        entry->data = 0x0;
        entry->len = 0;
    }
    //pthread_cond_broadcast(&entry->cond);
    return 0;
}

static struct arp_entry * arp_table_freespace (void) {
    struct arp_entry *entry;

    for (entry = arp_table; entry < array_tailof(arp_table); entry++) {
        if (!entry->used) {
            return entry;
        }
    }
    return 0x0;
}

static int arp_table_insert (const ip_addr_t *pa, const uint8 *ha) {
    struct arp_entry *entry;

    entry = arp_table_freespace();
    if (!entry) {
        return -1;
    }
    entry->used = 1;
    entry->pa = *pa;
    memcpy(entry->ha, ha, ETHERNET_ADDR_LEN);
    time(&entry->timestamp);
    //pthread_cond_broadcast(&entry->cond);
    return 0;
}

static void arp_entry_clear (struct arp_entry *entry) {
    entry->used = 0;
    entry->pa = 0;
    memset(entry->ha, 0, ETHERNET_ADDR_LEN);
    //entry->timestamp = 0;
    if (entry->data) {
        kfree(entry->data);
        entry->data = 0x0;
        entry->len = 0;
    }
    entry->netif = 0x0;
    /* !!! Don't touch entry->cond !!! */
}

static void arp_table_patrol (void) {
    struct arp_entry *entry;

    for (entry = arp_table; entry < array_tailof(arp_table); entry++) {
        if (entry->used && timestamp - entry->timestamp > ARP_TABLE_TIMEOUT_SEC) {
            arp_entry_clear(entry);
            //pthread_cond_broadcast(&entry->cond);
        }
    }
}

static int arp_send_request (struct netif *netif, const ip_addr_t *tpa) {
    struct arp_ethernet request;

    if (!tpa) {
        return -1;
    }
    request.hdr.hrd = hton16(ARP_HRD_ETHERNET);
    request.hdr.pro = hton16(ETHERNET_TYPE_IP);
    request.hdr.hln = ETHERNET_ADDR_LEN;
    request.hdr.pln = IP_ADDR_LEN;
    request.hdr.op = hton16(ARP_OP_REQUEST);
    memcpy(request.sha, netif->dev->addr, ETHERNET_ADDR_LEN);
    request.spa = ((struct netif_ip *)netif)->unicast;
    memset(request.tha, 0, ETHERNET_ADDR_LEN);
    request.tpa = *tpa;
#ifdef DEBUG
    fprintf(stderr, ">>> arp_send_request <<<\n");
    arp_dump((uint8 *)&request, sizeof(request));
#endif
    if (netif->dev->ops->xmit(netif->dev, ETHERNET_TYPE_ARP, (uint8 *)&request, sizeof(request), ETHERNET_ADDR_BROADCAST) == -1) {
        return -1;
    }
    return 0;
}

static int arp_send_reply (struct netif *netif, const uint8 *tha, const ip_addr_t *tpa, const uint8 *dst) {
    struct arp_ethernet reply;

    if (!tha || !tpa) {
        return -1;
    }
    reply.hdr.hrd = hton16(ARP_HRD_ETHERNET);
    reply.hdr.pro = hton16(ETHERNET_TYPE_IP);
    reply.hdr.hln = ETHERNET_ADDR_LEN;
    reply.hdr.pln = IP_ADDR_LEN;
    reply.hdr.op = hton16(ARP_OP_REPLY);
    memcpy(reply.sha, netif->dev->addr, ETHERNET_ADDR_LEN);
    reply.spa = ((struct netif_ip *)netif)->unicast;
    memcpy(reply.tha, tha, ETHERNET_ADDR_LEN);
    reply.tpa = *tpa;
#ifdef DEBUG
    cprintf(">>> arp_send_reply <<<\n");
    arp_dump((uint8 *)&reply, sizeof(reply));
#endif
    if (netif->dev->ops->xmit(netif->dev, ETHERNET_TYPE_ARP, (uint8 *)&reply, sizeof(reply), dst) < 0) {
        return -1;
    }
    return 0;
}

static void arp_rx (uint8 *packet, uint32 plen, struct netdev *dev) {
    struct arp_ethernet *message;
    time_t now;
    int marge = 0;
    struct netif *netif;

    if (plen < sizeof(struct arp_ethernet)) {
        return;
    }
    message = (struct arp_ethernet *)packet;
    if (ntoh16(message->hdr.hrd) != ARP_HRD_ETHERNET) {
        return;
    }
    if (ntoh16(message->hdr.pro) != ETHERNET_TYPE_IP) {
        return;
    }
    if (message->hdr.hln != ETHERNET_ADDR_LEN) {
        return;
    }
    if (message->hdr.pln != IP_ADDR_LEN) {
        return;
    }
#ifdef DEBUG
    cprintf(">>> arp_rx <<<\n");
    arp_dump(packet, plen);
#endif
    acquire(&arplock);
    time(&now);
    if (now - timestamp > 10) {
        timestamp = now;
        arp_table_patrol();
    }
    marge = (arp_table_update(dev, &message->spa, message->sha) == 0) ? 1 : 0;
    release(&arplock);
    netif = netdev_get_netif(dev, NETIF_FAMILY_IPV4);
    if (netif && ((struct netif_ip *)netif)->unicast == message->tpa) {
        if (!marge) {
            acquire(&arplock);
            arp_table_insert(&message->spa, message->sha);
            release(&arplock);
        }
        if (ntoh16(message->hdr.op) == ARP_OP_REQUEST) {
            arp_send_reply(netif, message->sha, &message->spa, message->sha);
        }
    }
    return;
}

int arp_resolve (struct netif *netif, const ip_addr_t *pa, uint8 *ha, const void *data, uint32 len) {
    struct arp_entry *entry;
    int ret;

    acquire(&arplock);
    entry = arp_table_select(pa);
    if (entry) {
        if (memcmp(entry->ha, ETHERNET_ADDR_ANY, ETHERNET_ADDR_LEN) == 0) {
            arp_send_request(netif, pa); /* just in case packet loss */
            release(&arplock);
            return ARP_RESOLVE_QUERY;
        }
        memcpy(ha, entry->ha, ETHERNET_ADDR_LEN);
        release(&arplock);
        return ARP_RESOLVE_FOUND;
    }
    entry = arp_table_freespace();
    if (!entry) {
        release(&arplock);
        return ARP_RESOLVE_ERROR;
    }
/*
    if (data) {
        entry->data = (uint8*)kalloc();
        if (!entry->data) {
            release(&arplock);
            return ARP_RESOLVE_ERROR;
        }
        memcpy(entry->data, data, len);
        entry->len = len;
    }
*/
    entry->used = 1;
    entry->pa = *pa;
    time(&entry->timestamp);
    entry->netif = netif;
    arp_send_request(netif, pa);
    release(&arplock);
    return ARP_RESOLVE_QUERY;
}

int arp_init (void) {
    struct arp_entry *entry;

    time(&timestamp);
    initlock(&arplock, "arp");
    netproto_register(NETPROTO_TYPE_ARP, arp_rx);
    return 0;
}
