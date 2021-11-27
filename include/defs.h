struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct spinlock;
struct stat;
struct superblock;

// Networking
struct pci_func;
struct netdev;
struct netif;
struct queue_head;
struct queue_entry;
struct socket;
struct sockaddr;

// arp.c
typedef uint32 ip_addr_t;
typedef int32  time_t;
int             arp_resolve(struct netif *netif, const ip_addr_t *pa, uint8 *ha, const void *data, uint32 len);
int             arp_init(void);

// e1000.c
int             e1000_init(struct pci_func *pcif);
void            e1000intr(void);

// ethernet.c
int             ethernet_addr_pton(const char *p, uint8 *n);
char *          ethernet_addr_ntop(const uint8 *n, char *p, uint32 size);
int32           ethernet_rx_helper(struct netdev *dev, uint8 *frame, uint32 flen, void (*cb)(struct netdev*, uint16, uint8*, uint32));
int32           ethernet_tx_helper(struct netdev *dev, uint16 type, const uint8 *payload, uint32 plen, const void *dst, int32 (*cb)(struct netdev*, uint8*, uint32));
void            ethernet_netdev_setup(struct netdev *dev);

// net.c
struct netdev * netdev_root(void);
struct netdev * netdev_alloc(void (*setup)(struct netdev *));
int             netdev_register(struct netdev *dev);
struct netdev * netdev_by_index(int index);
struct netdev * netdev_by_name(const char *name);
void            netdev_receive(struct netdev *dev, uint16 type, uint8 *packet, unsigned int plen);
int             netdev_add_netif(struct netdev *dev, struct netif *netif);
struct netif *  netdev_get_netif(struct netdev *dev, int family);
int             netproto_register(unsigned short type, void (*handler)(uint8 *packet, uint32 plen, struct netdev *dev));
void            netinit(void);

// ip.c
int             ip_addr_pton(const char *p, ip_addr_t *n);
char *          ip_addr_ntop(const ip_addr_t *n, char *p, uint32 size);
struct netif *  ip_netif_alloc(ip_addr_t unicast, ip_addr_t netmask, ip_addr_t gateway);
struct netif *  ip_netif_register(struct netdev *dev, const char *addr, const char *netmask, const char *gateway);
int             ip_netif_reconfigure(struct netif *netif, ip_addr_t unicast, ip_addr_t netmask, ip_addr_t gateway);
struct netif *  ip_netif_by_addr(ip_addr_t *addr);
struct netif *  ip_netif_by_peer(ip_addr_t *peer);
int32           ip_tx(struct netif *netif, uint8 protocol, const uint8 *buf, uint32 len, const ip_addr_t *dst);
int             ip_add_protocol(uint8 type, void (*handler)(uint8 *payload, uint32 len, ip_addr_t *src, ip_addr_t *dst, struct netif *netif));
int             ip_init(void);

// icmp.c
int             icmp_tx(struct netif *netif, uint8 type, uint8 code, uint32 values, uint8 *data, uint32 len, ip_addr_t *dst);
int             icmp_init(void);

// udp.c
int             udp_init(void);
int             udp_api_open(void);
int             udp_api_close(int soc);
int             udp_api_bind(int soc, struct sockaddr *addr, int addrlen);
int32           udp_api_recvfrom(int soc, uint8 *buf, uint32 size, struct sockaddr *addr, int *addrlen);
int32           udp_api_sendto(int soc, uint8 *buf, uint32 len, struct sockaddr *addr, int addrlen);

// tcp.c
int             tcp_init(void);
int             tcp_api_open(void);
int             tcp_api_close(int soc);
int             tcp_api_connect(int soc, struct sockaddr *addr, int addrlen);
int             tcp_api_bind(int soc, struct sockaddr *addr, int addrlen);
int             tcp_api_listen(int soc, int backlog);
int             tcp_api_accept(int soc, struct sockaddr *addr, int *addrlen);
int32           tcp_api_recv(int soc, uint8 *buf, uint32 size);
int32           tcp_api_send(int soc, uint8 *buf, uint32 len);

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);

// console.c
void            consoleinit(void);
void            cprintf(char*, ...);
void            consoleintr(int(*)(void));
void            panic(char*) __attribute__((noreturn));

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);
int             fileseek(struct file *f, int offset);

// fs.c
void            readsb(int dev, struct superblock *sb);
int             dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit(void);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint, uint);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);
extern uchar    ioapicid;
void            ioapicinit(void);

// kalloc.c
char*           kalloc(void);
char*           kmalloc(uint16 pages);
void            kfree(char*);
void            kinit1(void*, void*);
void            kinit2(void*, void*);

// kbd.c
void            kbdintr(void);

// lapic.c
int             cpunum(void);
extern volatile uint*    lapic;
void            lapiceoi(void);
void            lapicinit(void);
void            lapicstartap(uchar, uint);
void            microdelay(int);

// log.c
void            initlog(void);
void            log_write(struct buf*);
void            begin_op();
void            end_op();

// mp.c
extern int      ismp;
int             mpbcpu(void);
void            mpinit(void);
void            proclookinit();
void            mpstartthem(void);

// apic.c
int             acpiinit(void);

// picirq.c
void            picenable(int);
void            picinit(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

// proc.c
struct proc*    copyproc(struct proc*);
void            exit(void);
int             fork(void);
int             bfork(void);
int             growproc(int);
int             kill(int);
int             bless(int);
int             damn(int);
void            pinit(void);
void            procdump(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(void);
void            wakeup(void*);
void            yield(void);
enum procstate  pstate(int);
int             pname(int, char*, int);

// swtch.S
void            swtch(struct context**, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
void            getcallerpcs(void*, uintp*);
void            getstackpcs(uintp*, uintp*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            pushcli(void);
void            popcli(void);

// syscall.c
int             argint(int, int*);
int             arglong(int n, long* lp);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             arguintp(int, uintp*);
int             fetchuintp(uintp, uintp*);
int             fetchstr(uintp, char**);
void            syscall(void);

// timer.c
void            timerinit(void);

// trap.c
void            idtinit(void);
extern uint     ticks;
void            tvinit(void);
extern struct spinlock tickslock;

// uart.c
#define COM1      0x3F8
#define COM2      0x2F8
#define COM3      0x3E8
#define COM4      0x2E8
void		uartearlyinit(void);
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);

// vm.c
void            seginit(void);
void            kvmalloc(void);
void            vmenable(void);
pde_t*          setupkvm(void);
char*           uva2ka(pde_t*, char*);
int             allocuvm(pde_t*, uint, uint);
int             deallocuvm(pde_t*, uintp, uintp);
void            freevm(pde_t*);
void            inituvm(pde_t*, char*, uint);
int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
pde_t*          copyuvm(pde_t*, uint);
void            switchuvm(struct proc*);
void            switchkvm(void);
int             copyout(pde_t*, uint, void*, uint);
void            clearpteu(pde_t *pgdir, char *uva);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

//useful for when debugging - clearly/easily stop execution at a certain point
#define STOP while(1){}
