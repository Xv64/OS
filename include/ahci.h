#include "types.h"
#include "defs.h"

#define AHCI_MAX_BUS  0xFF
#define AHCI_MAX_SLOT 0x1F
#define AHCI_HBA_PORT 0xCF8 //???

#define AHCI_VENDOR_OFFSET 0x0
#define AHCI_DEVICE_OFFSET 0x02

//AHCI vendors:
#define AHCI_VENDOR_INTEL 0x8086

//AHCI devices:
#define AHCI_ICH9 0x2922

void ahci_init();
ushort ahci_probe(ushort bus, ushort slot, ushort offset);
