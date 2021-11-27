#define IP_HDR_SIZE_MIN 20
#define IP_HDR_SIZE_MAX 60
#define IP_PAYLOAD_SIZE_MAX (65535 - IP_HDR_SIZE_MIN)

#define IP_ADDR_LEN 4
#define IP_ADDR_STR_LEN 16 /* "ddd.ddd.ddd.ddd\0" */

#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_TCP  0x06
#define IP_PROTOCOL_UDP  0x11
#define IP_PROTOCOL_RAW  0xff

struct netif_ip {
    struct netif netif;
    ip_addr_t unicast;
    ip_addr_t netmask;
    ip_addr_t network;
    ip_addr_t broadcast;
    ip_addr_t gateway;
};
