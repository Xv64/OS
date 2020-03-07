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


//The following structs are as documented @ https://wiki.osdev.org/AHCI
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
	char  cfl:5;    // Command FIS length in DWORDS, 2 ~ 16
	char  a:1;      // ATAPI
	char  w:1;      // Write, 1: H2D, 0: D2H
	char  p:1;      // Prefetchable

	char  r:1;      // Reset
	char  b:1;      // BIST
	char  c:1;      // Clear busy upon R_OK
	char  rsv0:1;   // Reserved
	char  pmp:4;    // Port multiplier port

	ushort prdtl;   // Physical region descriptor table length in entries

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
	char  fis_type;  // FIS_TYPE_REG_H2D

	char  pmport:4;  // Port multiplier
	char  rsv0:3;    // Reserved
	char  c:1;       // 1: Command, 0: Control

	char  command;   // Command register
	char  featurel;  // Feature register, 7:0

	// DWORD 1
	char  lba0;      // LBA low register, 7:0
	char  lba1;      // LBA mid register, 15:8
	char  lba2;      // LBA high register, 23:16
	char  device;    // Device register

	// DWORD 2
	char  lba3;     // LBA register, 31:24
	char  lba4;     // LBA register, 39:32
	char  lba5;     // LBA register, 47:40
	char  featureh; // Feature register, 15:8

	// DWORD 3
	char  countl;   // Count register, 7:0
	char  counth;   // Count register, 15:8
	char  icc;      // Isochronous command completion
	char  control;  // Control register

	// DWORD 4
	char  rsv1[4];  // Reserved
} FIS_REG_H2D;

typedef struct tagHBA_PRDT_ENTRY {
	uint32 dba;    // Data base address
	uint32 dbau;   // Data base address upper 32 bits
	uint32 rsv0;   // Reserved

	// DW3
	uint32 dbc:22; // Byte count, 4M max
	uint32 rsv1:9; // Reserved
	uint32 i:1;   // Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL {
	// 0x00
	char cfis[64]; // Command FIS

	// 0x40
	char acmd[16]; // ATAPI command, 12 or 16 bytes

	// 0x50
	char rsv[48];  // Reserved

	// 0x80
	HBA_PRDT_ENTRY prdt_entry[1];  // Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;
