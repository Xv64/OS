#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_PROG_IF 0x09
#define PCI_SUBCLASS_ADDR 0x0a
#define PCI_CLASS_ADDR 0x0b
#define PCI_CACHE_LINE_SIZE 0x0c
#define PCI_LATENCY_TIMER 0x0d
#define PCI_HEADER_TYPE 0x0e
#define PCI_BIST_ADDR 0x0f
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_INTERRUPT_LINE_ADDR 0x3C
#define PCI_SECONDARY_BUS 0x09
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#ifndef XV64_PCI_H
#define XV64_PCI_H

void pciinit(void);

// PCI subsystem interface
enum { pci_res_bus, pci_res_mem, pci_res_io, pci_res_max };

struct pci_bus;

struct pci_func {
    struct pci_bus *bus;	// Primary bus for bridges

    uint32 dev;
    uint32 func;

    uint32 dev_id;
    uint32 dev_class;

    uint32 reg_base[6];
    uint32 reg_size[6];
    uint8 irq_line;
};

struct pci_dev {
	uint32 register_offset;
	uint32 function_num;
	uint32 device_num;
	uint32 bus_num;
	uint32 reserved;
	uint32 enable;
};

struct pci_bus {
    struct pci_func *parent_bridge;
    uint32 busno;
};

uint32 pci_get_device_type(struct pci_dev dev);
uint32 dev_pci_read(struct pci_dev dev, uint32 field);
void dev_pci_write(struct pci_dev dev, uint32 field, uint32 value);
struct pci_dev dev_pci_get_device(uint16 vendor_id, uint16 device_id, uint32 device_type);


#endif
