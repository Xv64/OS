#include "ahci.h"
#include "types.h"
#include "defs.h"
#include "x86.h"

//AHCI implementation.
//See Intel reference docs @ https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

static const struct {
    uint16	vendor;
    uint16	device;
    const char	*name;
} ahci_devices[] = {
    {AHCI_VENDOR_INTEL, 0x27D0, "Intel ICH7 Port 1"},
    {AHCI_VENDOR_INTEL, 0x27D2, "Intel ICH7 Port 2"},
    //{AHCI_VENDOR_INTEL, 0x244E, "Intel 82801 PCI Bridge"},
    {AHCI_VENDOR_INTEL, 0x2829, "Intel ICH8M"},
    {AHCI_VENDOR_INTEL, 0x2922, "Intel ICH9"},
    {AHCI_VENDOR_INTEL, 0x1E03, "Intel Panther Point"},
    {AHCI_VENDOR_VMWARE, 0x07E0, "VMWare SATA"},
    {AHCI_VENDOR_VMWARE, 0x07A0, "VMWare PCIE Root"},
    {0, 0, ""} //this is a terminal node - this must be present and the last entry
};

void ahci_init(){
    cprintf("probing AHCI...\n");

    for(uint16 bus = 0; bus <= AHCI_MAX_BUS; bus++) {
        for(uint16 slot = 0; slot <= AHCI_MAX_SLOT; slot++) {
            for(uint16 func = 0; func <= AHCI_MAX_FUNC; func++){
                ahci_try_setup_device(bus, slot, func);
            }
        }
    }
}

void ahci_try_setup_device(uint16 bus, uint16 slot, uint16 func) {
    uint16 vendor = ahci_probe(bus, slot, func, AHCI_VENDOR_OFFSET);
    uint16 device = ahci_probe(bus, slot, func, AHCI_DEVICE_OFFSET);

    uint64 ahci_base_mem = ahci_read(bus, slot, func, AHCI_BAR5_OFFSET); //find ABAR

    if(ahci_base_mem != 0 && ahci_base_mem != 0xffffffff) {
        //we found something, but what?
        const char *name;
        int identified = 0;
        for(uint16 i = 0; ahci_devices[i].vendor != 0; i++){
            if(ahci_devices[i].vendor == vendor && ahci_devices[i].device == device){
                name = ahci_devices[i].name;
                identified = 1;
            }
        }
        if(identified){
            ahci_try_setup_known_device(name, ahci_base_mem, bus, slot, func);
        }else{
            cprintf("unknown device found (bus=%d, slot=%d, func=%d, abar=0x%x, vendor=0x%x, device=0x%x)\n", bus, slot, func, ahci_base_mem, vendor, device);
        }
    }
}

void ahci_try_setup_known_device(char *dev_name, uint64 ahci_base_mem, uint16 bus, uint16 slot, uint16 func) {
    cprintf("%s controller found (bus=%d, slot=%d, func=%d, abar=0x%x)\n", dev_name, bus, slot, func, ahci_base_mem);

    HBA_MEM *ptr = (HBA_MEM *)&ahci_base_mem;
    cprintf(" HBA in ");
    if(ptr->ghc == 0x0){
        cprintf("legacy mode\n");
    }else{
        cprintf("AHCI-only mode\n");
    }

    uint64 pi = ptr->pi;
    for(int i = 0; (i != 32); i++){
        uint64 port_mask = 1 << i;
        if((pi & port_mask) == 0x0){
            continue;
        }

        HBA_PORT *hba_port = (HBA_PORT *) &ptr->ports[i];

        if(hba_port->sig != SATA_SIG_ATAPI && hba_port->sig != SATA_SIG_SEMB && hba_port->sig != SATA_SIG_PM){
            //we may have found a SATA device, but what is the status of this device?
            uint64 ssts = amd64_spinread64(&hba_port->ssts, 0);

            uint8 ipm = (ssts >> 8) & 0x0F;
            uint8 spd = (ssts >> 4) & 0x0F;
            uint8 det = ssts & 0x7; //the Device Detection (DET) flags are the bottom 3 bits

            if (det != HBA_PORT_DET_PRESENT && ipm != HBA_PORT_IPM_ACTIVE){
                //nope
		 	}else if(hba_port->sig==SATA_SIG_ATAPI){
                //ATAPI device
			}else if(hba_port->sig==SATA_SIG_SEMB){

			}else if(hba_port->sig==SATA_SIG_PM){
                //port multiplier detected
			}else{
                cprintf("SATA device detected:\n");
                cprintf("   port[%d].sig = %x\n", i, hba_port->sig);
                cprintf("   ipm=%x, spd=%x, det=%x\n", ipm, spd, det);
                ahci_sata_init(hba_port, i);
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

int ahci_sata_read(HBA_PORT *port, uint32 startl, uint32 starth, uint32 count, uint16 *buf) {
    return 0;
}

void ahci_sata_init(HBA_PORT *port, int num){
	ahci_rebase_port(port,num);
    //TODO
}

void ahci_rebase_port(HBA_PORT *port, int num) {

    if(!ahci_stop_port(port)){
        cprintf("   unable to stop command engine, skipping HBA\n");
        return;
    }
    //TODO
}

uint16 ahci_stop_port(HBA_PORT *port) {
    //TODO set port->FBU "with a valid pointer to the FIS receive area"
	port->cmd |= HBA_PxCMD_FRE; //System software must not set this bit until PxFB (PxFBU) have been programmed with a valid pointer to the FIS receive area... (SATA/AHCI spec 1.3.1, section 3.3.7)
	port->cmd &= ~HBA_PxCMD_ST; //This bit shall only be set to ‘1’ by software after PxCMD.FRE has been set to ‘1’ (SATA/AHCI spec 1.3.1, section 3.3.7)

    uint64 cmd;
    uint16 count = 0;
    do { // Wait until FR (bit14), CR (bit15) are cleared
        cmd = amd64_spinread64(&port->cmd, 0);
        if(!(port->cmd & (HBA_PxCMD_CR | HBA_PxCMD_FR)))
            break;
    }while(count++ < 1000);

    if(count >= 1000){
        return 0;
    }

	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
    return 1;
}

void ahci_start_port(HBA_PORT *port) {
	//TODO
}
