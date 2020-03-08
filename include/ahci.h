#include "types.h"
#include "defs.h"

#define AHCI_MAX_BUS  0xFF
#define AHCI_MAX_SLOT 0x1F
#define AHCI_MAX_FUNC 0x7
#define AHCI_HBA_PORT 0xCF8 //???

#define FIS_TYPE_REG_H2D 0x27
#define ATA_CMD_READ_DMA_EX 0x25

#define AHCI_VENDOR_OFFSET 0x0
#define AHCI_DEVICE_OFFSET 0x02
//...
#define AHCI_BAR0_OFFSET   0x10
#define AHCI_BAR1_OFFSET   0x14
#define AHCI_BAR2_OFFSET   0x18
#define AHCI_BAR3_OFFSET   0x1C
#define AHCI_BAR4_OFFSET   0x20
#define AHCI_BAR5_OFFSET   0x24

//AHCI vendors:
#define AHCI_VENDOR_INTEL 0x8086

//AHCI devices:
#define AHCI_ICH9 0x2922


//The following structs are as documented @ https://wiki.osdev.org/AHCI
//START
typedef volatile struct tagHBA_PORT {
	uint32 clb;		   // 0x00, command list base address, 1K-byte aligned
	uint32 clbu;	   // 0x04, command list base address upper 32 bits
	uint32 fb;         // 0x08, FIS base address, 256-byte aligned
	uint32 fbu;        // 0x0C, FIS base address upper 32 bits
	uint32 is;         // 0x10, interrupt status
	uint32 ie;         // 0x14, interrupt enable
	uint32 cmd;        // 0x18, command and status
	uint32 rsv0;       // 0x1C, Reserved
	uint32 tfd;        // 0x20, task file data
	uint32 sig;        // 0x24, signature
	uint32 ssts;       // 0x28, SATA status (SCR0:SStatus)
	uint32 sctl;       // 0x2C, SATA control (SCR2:SControl)
	uint32 serr;       // 0x30, SATA error (SCR1:SError)
	uint32 sact;       // 0x34, SATA active (SCR3:SActive)
	uint32 ci;         // 0x38, command issue
	uint32 sntf;       // 0x3C, SATA notification (SCR4:SNotification)
	uint32 fbs;        // 0x40, FIS-based switch control
	uint32 rsv1[11];   // 0x44 ~ 0x6F, Reserved
	uint32 vendor[4];  // 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef struct tagHBA_CMD_HEADER {
	// DW0
	uint8  cfl:5;    // Command FIS length in DWORDS, 2 ~ 16
	uint8  a:1;      // ATAPI
	uint8  w:1;      // Write, 1: H2D, 0: D2H
	uint8  p:1;      // Prefetchable

	uint8  r:1;      // Reset
	uint8  b:1;      // BIST
	uint8  c:1;      // Clear busy upon R_OK
	uint8  rsv0:1;   // Reserved
	uint8  pmp:4;    // Port multiplier port

	uint16 prdtl;   // Physical region descriptor table length in entries

	// DW1
	volatile
	uint32 prdbc;   // Physical region descriptor byte count transferred

	// DW2, 3
	uint32 ctba;    // Command table descriptor base address
	uint32 ctbau;   // Command table descriptor base address upper 32 bits

	// DW4 - 7
	uint32 rsv1[4]; // Reserved
} HBA_CMD_HEADER;

typedef struct tagFIS_REG_H2D {
	// DWORD 0
	uint8  fis_type;  // FIS_TYPE_REG_H2D

	uint8  pmport:4;  // Port multiplier
	uint8  rsv0:3;    // Reserved
	uint8  c:1;       // 1: Command, 0: Control

	uint8  command;   // Command register
	uint8  featurel;  // Feature register, 7:0

	// DWORD 1
	uint8  lba0;      // LBA low register, 7:0
	uint8  lba1;      // LBA mid register, 15:8
	uint8  lba2;      // LBA high register, 23:16
	uint8  device;    // Device register

	// DWORD 2
	uint8  lba3;     // LBA register, 31:24
	uint8  lba4;     // LBA register, 39:32
	uint8  lba5;     // LBA register, 47:40
	uint8  featureh; // Feature register, 15:8

	// DWORD 3
	uint8  countl;   // Count register, 7:0
	uint8  counth;   // Count register, 15:8
	uint8  icc;      // Isochronous command completion
	uint8  control;  // Control register

	// DWORD 4
	uint8  rsv1[4];  // Reserved
} FIS_REG_H2D;

typedef struct tagHBA_PRDT_ENTRY {
	uint32 dba;    // Data base address
	uint32 dbau;   // Data base address upper 32 bits
	uint32 rsv0;    // Reserved

	// DW3
	uint32 dbc:22; // Byte count, 4M max
	uint32 rsv1:9; // Reserved
	uint32 i:1;   // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL {
	// 0x00
	uint8 cfis[64]; // Command FIS

	// 0x40
	uint8 acmd[16]; // ATAPI command, 12 or 16 bytes

	// 0x50
	uint8 rsv[48];  // Reserved

	// 0x80
	HBA_PRDT_ENTRY prdt_entry[1];  // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef volatile struct tagHBA_MEM
{
	// 0x00 - 0x2B, Generic Host Control
	uint32 cap;		// 0x00, Host capability
	uint32 ghc;		// 0x04, Global host control
	uint32 is;		// 0x08, Interrupt status
	uint32 pi;		// 0x0C, Port implemented
	uint32 vs;		// 0x10, Version
	uint32 ccc_ctl;	// 0x14, Command completion coalescing control
	uint32 ccc_pts;	// 0x18, Command completion coalescing ports
	uint32 em_loc;		// 0x1C, Enclosure management location
	uint32 em_ctl;		// 0x20, Enclosure management control
	uint32 cap2;		// 0x24, Host capabilities extended
	uint32 bohc;		// 0x28, BIOS/OS handoff control and status

	// 0x2C - 0x9F, Reserved
	uint8  rsv[0xA0-0x2C];

	// 0xA0 - 0xFF, Vendor specific registers
	uint8  vendor[0x100-0xA0];

	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[1];	// 1 ~ 32
} HBA_MEM;
//END

void ahci_init();
uint16 ahci_probe(uint16 bus, uint16 slot, uint16 func, uint16 offset);
uint64 ahci_read(uint16 bus, uint16 slot, uint16 func, uint16 offset);
