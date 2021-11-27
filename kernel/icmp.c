// Copyright (c) 2012-2020 YAMAMOTO Masaya
// SPDX-License-Identifier: MIT

#include "types.h"
#include "defs.h"
#include "net.h"
#include "ip.h"
#include "icmp.h"
#include "kernel/klib.h"

struct icmp_hdr {
    uint8 type;
    uint8 code;
    uint16 sum;
    union {
        struct {
            uint16 id;
            uint16 seq;
        } echo;
        ip_addr_t gateway;
        uint8 ptr;
        uint32 unused;
    } un;
#define ih_ptr     un.ptr
#define ih_id      un.echo.id
#define ih_seq     un.echo.seq
#define ih_unused  un.unused
#define ih_values  un.unused
#define ih_gateway un.gateway
    uint8 data[0];
};

#define ICMP_BUFSIZ IP_PAYLOAD_SIZE_MAX

// static char *icmp_type_ntoa (uint8 type) {
//     switch (type) {
//     case ICMP_TYPE_ECHOREPLY:
//         return "Echo Reply";
//     case ICMP_TYPE_DEST_UNREACH:
//         return "Destination Unreachable";
//     case ICMP_TYPE_SOURCE_QUENCH:
//         return "Source Quench";
//     case ICMP_TYPE_REDIRECT:
//         return "Redirect";
//     case ICMP_TYPE_ECHO:
//         return "Echo";
//     case ICMP_TYPE_TIME_EXCEEDED:
//         return "Time Exceeded";
//     case ICMP_TYPE_PARAM_PROBLEM:
//         return "Parameter Problem";
//     case ICMP_TYPE_TIMESTAMP:
//         return "Timestamp";
//     case ICMP_TYPE_TIMESTAMPREPLY:
//         return "Timestamp Reply";
//     case ICMP_TYPE_INFO_REQUEST:
//         return "Information Request";
//     case ICMP_TYPE_INFO_REPLY:
//         return "Information Reply";
//     }
//     return "UNKNOWN";
// }

static void icmp_rx (uint8 *packet, uint32 plen, ip_addr_t *src, ip_addr_t *dst, struct netif *netif) {
    struct icmp_hdr *hdr;

    (void)dst;
    if (plen < sizeof(struct icmp_hdr)) {
        return;
    }
    hdr = (struct icmp_hdr *)packet;
    switch (hdr->type) {
    case ICMP_TYPE_ECHO:
        icmp_tx(netif, ICMP_TYPE_ECHOREPLY, hdr->code, hdr->ih_values, hdr->data, plen - sizeof(struct icmp_hdr), src);
        break;
    }
}

int icmp_tx (struct netif *netif, uint8 type, uint8 code, uint32 values, uint8 *data, uint32 len, ip_addr_t *dst) {
    uint8 buf[ICMP_BUFSIZ];
    struct icmp_hdr *hdr;
    uint32 msg_len;

    hdr = (struct icmp_hdr *)buf;
    hdr->type = type;
    hdr->code = code;
    hdr->sum = 0;
    hdr->ih_values = values;
    memcpy(hdr->data, data, len);
    msg_len = sizeof(struct icmp_hdr) + len;
    hdr->sum = cksum16((uint16 *)hdr, msg_len, 0);
    return ip_tx(netif, IP_PROTOCOL_ICMP, (uint8 *)hdr, msg_len, dst);
}

int icmp_init (void) {
    ip_add_protocol(IP_PROTOCOL_ICMP, icmp_rx);
    return 0;
}
