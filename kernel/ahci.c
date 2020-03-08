#include "ahci.h"
#include "types.h"
#include "defs.h"
#include "x86.h"

//AHCI implementation.
//See Intel reference docs @ https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

void ahci_init(){
    cprintf("probing AHCI...\n");

    for(uint16 bus = 0; bus <= AHCI_MAX_BUS; bus++) {
        for(uint16 slot = 0; slot <= AHCI_MAX_SLOT; slot++) {
            for(uint16 func = 0; func <= AHCI_MAX_FUNC; func++){
                uint16 vendor = ahci_probe(bus, slot, func, AHCI_VENDOR_OFFSET);
                uint16 device = ahci_probe(bus, slot, func, AHCI_DEVICE_OFFSET);

                uint64 ahci_base_mem = ahci_read(bus, slot, func, AHCI_BAR5_OFFSET); //find ABAR

                if(vendor == AHCI_VENDOR_INTEL && device == AHCI_ICH9){
                    cprintf("Intel ICH9 controller found (bus=%d, slot=%d, func=%d, abar=0x%x)\n", bus, slot, func, ahci_base_mem);

                    HBA_MEM* ptr = (HBA_MEM *) &ahci_base_mem;

                    for(int i = 0; i != 32; i++){
                        HBA_PORT *hba_port = (HBA_PORT *) &ptr->ports[i];

                        cprintf("\tport[%d].ssts = %x\n", i, hba_port->ssts);
                    }

                } else if(ahci_base_mem != 0 && ahci_base_mem != 0xffffffff) {
                    cprintf("unknown device found (bus=%d, slot=%d, func=%d, abar=0x%x, vendor=0x%x, device=0x%x)\n", bus, slot, func, ahci_base_mem, vendor, device);
                }
            }
        }
    }
}

ushort ahci_probe(ushort bus, ushort slot, uint16 func, ushort offset){

    uint32 lbus = (uint32)bus;
    uint32 lfunc = (uint32)func;
    uint32 lslot = (uint32)slot;

    //build 32-bit address for probe
    uint32 address = (
            (lbus <<  0x10)
          | (lslot << 0xB)
          | (lfunc << 8)
          | (offset & 0xFC)
          | ((uint32) 0x80000000)
      );
    amd64_outl(AHCI_HBA_PORT, address);

    uint32 register_value = amd64_inl(0xCFC);

    return (ushort) ((register_value >> ((offset & 2) * 8)) & 0xFFFF);
}

uint64 ahci_read(ushort bus, ushort slot,ushort func, ushort offset){

    uint32 lbus = (uint32)bus;
    uint32 lslot = (uint32)slot;
    uint32 lfunc = (uint32)func;
    uint64 tmp = 0;


    uint32 address = (uint32)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32)0x80000000));
    amd64_outl(AHCI_HBA_PORT, address);
    tmp = (uint64)(amd64_inl (0xCFC) /* & 0xffff*/);
    return (tmp);
}
