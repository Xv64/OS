#include "ahci.h"
#include "types.h"
#include "defs.h"
#include "x86.h"

//AHCI implementation.
//See Intel reference docs @ https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

void ahci_init(){
    cprintf("probing AHCI...\n");

    for(int bus = 0; bus <= AHCI_MAX_BUS; bus++) {
        for(int slot = 0; slot <= AHCI_MAX_SLOT; slot++) {
            ushort vendor = ahci_probe(bus, slot, AHCI_VENDOR_OFFSET);
            ushort device = ahci_probe(bus, slot, AHCI_DEVICE_OFFSET);

            if(vendor == AHCI_VENDOR_INTEL && device == AHCI_ICH9){
                cprintf("Intel ICH9 controller found (bus=%d, slot=%d)\n", bus, slot);
            }
        }
    }
}

ushort ahci_probe(ushort bus, ushort slot, ushort offset){

    uint32 lbus = (uint32)bus;
    uint32 lslot = (uint32)slot;

    //build 32-bit address for probe
    uint32 address = (
            (lbus <<  0x10)
          | (lslot << 0xB)
          | (offset & 0xFC)
          | ((uint32) 0x80000000)
      );
    amd64_outl(AHCI_HBA_PORT, address);

    uint32 register_value = amd64_inl(0xCFC);
    return (ushort) ((register_value >> ((offset & 2) * 8)) & 0xFFFF);
 }
