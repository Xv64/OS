#ifndef JOS_KERN_PCI_H
#define JOS_KERN_PCI_H

void pciinit(void);

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

#endif
