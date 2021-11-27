#define NETDEV_TYPE_ETHERNET  (0x0001)
#define NETDEV_TYPE_SLIP      (0x0002)

#define NETDEV_FLAG_BROADCAST 0x0002
#define NETDEV_FLAG_MULTICAST 0x8000
#define NETDEV_FLAG_P2P       0x0010
#define NETDEV_FLAG_LOOPBACK  0x0008
#define NETDEV_FLAG_NOARP     0x0080
#define NETDEV_FLAG_PROMISC   0x0100
#define NETDEV_FLAG_RUNNING   0x0040
#define NETDEV_FLAG_UP        0x0001

#define NETPROTO_TYPE_IP      (0x0800)
#define NETPROTO_TYPE_ARP     (0x0806)
#define NETPROTO_TYPE_IPV6    (0x86dd)

#define NETIF_FAMILY_IPV4     (0x02)
#define NETIF_FAMILY_IPV6     (0x0a)

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

struct netdev;

struct netif {
    struct netif *next;
    uint8 family;
    struct netdev *dev;
    /* Depends on implementation of protocols. */
};

struct netdev_ops {
    int (*open)(struct netdev *dev);
    int (*stop)(struct netdev *dev);
    int (*xmit)(struct netdev *dev, uint16 type, const uint8 *packet, uint32 size, const void *dst);
};

struct netdev {
    struct netdev *next;
    struct netif *ifs;
    int index;
    char name[IFNAMSIZ];
    uint16 type;
    uint16 mtu;
    uint16 flags;
    uint16 hlen;
    uint16 alen;
    uint8 addr[16];
    uint8 peer[16];
    uint8 broadcast[16];
    struct netdev_ops *ops;
    void *priv;
};
