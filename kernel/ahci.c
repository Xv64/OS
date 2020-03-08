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
            uint16 vendor = ahci_probe(bus, slot, AHCI_VENDOR_OFFSET);
            uint16 device = ahci_probe(bus, slot, AHCI_DEVICE_OFFSET);

            if(vendor == AHCI_VENDOR_INTEL && device == AHCI_ICH9){
                cprintf("Intel ICH9 controller found (bus=%d, slot=%d)\n", bus, slot);

                uint64 ahci_base_mem = ahci_read(bus, slot, 0, AHCI_BAR5_OFFSET); //find ABAR
                cprintf("\t[base mem @ %x]\n", ahci_base_mem);

                HBA_MEM* ptr = (HBA_MEM *)(&ahci_base_mem);

                cprintf("\tcap: %x\n", ptr->cap);
                cprintf("\tghc: %x\n", ptr->ghc);
                cprintf("\tis: %x\n", ptr->is);
                cprintf("\tpi: %x\n", ptr->pi);
                cprintf("\tvs: %x\n", ptr->vs);
                for(int i = 0; i != 32; i++){
                    HBA_PORT *hba_port = (HBA_PORT *)&ptr->ports[i];
                    cprintf("\tport[%d].ssts = %x\n", i, hba_port->ssts);
                }

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

uint64 ahci_read(uint16 bus, uint16 slot, uint16 func, uint16 offset){
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
