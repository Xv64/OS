#ifndef XV64_PCI_H
#define XV64_PCI_H

// PCI subsystem interface
enum { pci_res_bus, pci_res_mem, pci_res_io, pci_res_max };

struct pci_bus;

struct pci_func {
    struct pci_bus* bus;    // Primary bus for bridges

    uint32 dev;
    uint32 func;

    uint32 dev_id;
    uint32 dev_class;

    uint32 reg_base[6];
    uint32 reg_size[6];
    uint8 irq_line;
};

struct pci_bus {
    struct pci_func* parent_bridge;
    uint32 busno;
};

#define PCI_DEV_CLASS_STORAGE       0x1
#define PCI_DEV_CLASS_NETWORKING    0x2
#define PCI_DEV_CLASS_DISPLAY       0x3
#define PCI_DEV_CLASS_MULTIMEDIA    0x4
#define PCI_DEV_CLASS_MEMCONTROLLER 0x5
#define PCI_DEV_CLASS_BRIDGE        0x6

#define	PCI_SUBCLASS_BRIDGE_PCI 0x04

#define PCI_BAR0_OFFSET   0x10
#define PCI_BAR1_OFFSET   0x14
#define PCI_BAR2_OFFSET   0x18
#define PCI_BAR3_OFFSET   0x1C
#define PCI_BAR4_OFFSET   0x20
#define PCI_BAR5_OFFSET   0x24

#define PCI_ID_REG             0x00
#define PCI_COMMAND_STATUS_REG 0x04
#define PCI_CLASS_REG          0x08
#define	PCI_BHLC_REG           0x0C
#define PCI_BRIDGE_BUS_REG     0x18
#define PCI_BRIDGE_STATIO_REG  0x1C
#define PCI_INTERRUPT_REG      0x3C

#define PCI_CMD_IO_ENABLE     0x1
#define PCI_CMD_MEM_ENABLE    0x2
#define PCI_CMD_MASTER_ENABLE 0x4

#define PCI_BRIDGE_BUS_SECONDARY_SHIFT   0x08
#define PCI_BRIDGE_BUS_SUBORDINATE_SHIFT 0x10

#define PCI_MAPREG_TYPE_MEM       0x0
#define PCI_MAPREG_MEM_TYPE_64BIT 0x4


#define	PCI_CLASS(c) (((c) >> 0x18) & 0xFF)
#define	PCI_SUBCLASS(c) (((c) >> 0x10) & 0xFF)
#define	PCI_VENDOR(v) (v & 0xFFFF)
#define	PCI_PRODUCT(p) (((p) >> 0x10) & 0xFFFF)
#define	PCI_INTERRUPT_LINE(l) (l & 0xFF)
#define PCI_BRIDGE_IO_32BITS(r) ((r & 0xF) == 1)

#define	PCI_HDRTYPE(bhlcr) (((bhlcr) >> 0x10) & 0xFF)
#define	PCI_HDRTYPE_TYPE(bhlcr) (PCI_HDRTYPE(bhlcr) & 0x7F)
#define	PCI_HDRTYPE_MULTIFN(bhlcr) ((PCI_HDRTYPE(bhlcr) & 0x80) != 0)

#define PCI_MAPREG_NUM(reg) (((unsigned)(reg)-0x10) / 0x4)
#define	PCI_MAPREG_TYPE(reg) (reg & 0x1)
#define	PCI_MAPREG_MEM_TYPE(reg) (reg & 0x6)
#define	PCI_MAPREG_MEM_ADDR(reg) (reg & 0xFFFFFFF0)
#define	PCI_MAPREG_MEM_SIZE(reg) (PCI_MAPREG_MEM_ADDR(reg) & -PCI_MAPREG_MEM_ADDR(reg))
#define	PCI_MAPREG_IO_ADDR(reg) (reg & 0xFFFFFFFC)
#define	PCI_MAPREG_IO_SIZE(reg) (PCI_MAPREG_IO_ADDR(reg) & -PCI_MAPREG_IO_ADDR(reg))

void pciinit(void);

#endif
