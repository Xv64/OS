// See LICENSE for license details.

#ifndef _CPUDEF_H_
#define _CPUDEF_H_

/* cr0 */
#define CR0_PG                (1 << 31)       /* Paging */
#define CR0_CD                (1 << 30)       /* Cache Disable */
#define CR0_NW                (1 << 29)       /* Not Write-through */
#define CR0_AM                (1 << 18)       /* Alignment Mask */
#define CR0_WP                (1 << 16)       /* Write Protect */
#define CR0_NE                (1 << 5)        /* Numeric Error enable */
#define CR0_ET                (1 << 4)        /* Extension Type */
#define CR0_TS                (1 << 3)        /* Task Switched */
#define CR0_EM                (1 << 2)        /* Emulation */
#define CR0_MP                (1 << 1)        /* Monitor Coprocessor */
#define CR0_PE                (1 << 0)        /* Protected mode Enable */

/* cr2 */
#define CR2_PFLA              -1L             /* 63:0  Page Fault Linear Address */

/* cr3 */
#define CR3_PTPA              0xffffffffff000 /* 51:12  Page Table Physical Address */
#define CR3_PCD               (1 << 4)        /* Page-level Cache Disable */
#define CR3_PWT               (1 << 3)        /* Page-level Write-Through */

/* cr4 */
#define CR4_PKE               (1 << 22)       /* Protection-Key-Enable */
#define CR4_SMAP              (1 << 21)       /* Supervisor Mode Access Protection */
#define CR4_SMEP              (1 << 20)       /* Supervisor Mode Execute Protection */
#define CR4_OSXSAVE           (1 << 18)       /* XSAVE and Extended States-Enable */
#define CR4_PCIDE             (1 << 17)       /* PCID Enable */
#define CR4_FSGS              (1 << 16)       /* FSGSBASE Enable */
#define CR4_SMXE              (1 << 14)       /* Safer Mode Extensions Enable */
#define CR4_VMXE              (1 << 13)       /* Virtual Machine Extensions Enable */
#define CR4_LA57              (1 << 12)       /* Five-Level Paging */
#define CR4_UMIP              (1 << 11)       /* User-Mode Instruction Prevention */
#define CR4_OSXMMEXCPT        (1 << 10)       /* Unmasked SIMD FP Exceptions */
#define CR4_OSFXSR            (1 << 9)        /* FXSAVE and FXRSTOR Enable */
#define CR4_PCE               (1 << 8)        /* Performance Counter Enable */
#define CR4_PGE               (1 << 7)        /* Page Global Enable */
#define CR4_MCE               (1 << 6)        /* Machine-Check Enable */
#define CR4_PAE               (1 << 5)        /* Physical Address Extensions */
#define CR4_PSE               (1 << 4)        /* Page Size Extensions */
#define CR4_DE                (1 << 3)        /* Debugging Extensions */
#define CR4_TSD               (1 << 2)        /* Time Stamp Disable */
#define CR4_PVI               (1 << 1)        /* Protected-Mode Virtual Interrupts */
#define CR4_VME               (1 << 0)        /* Virtual-8086 Mode Extensions */

/* cr8 */
#define CR8_TPL               0x0000000f      /* Task Priority Level [0:3] */

/* ia32-efer */
#define IA32_EFER_NXE         (1 << 11)       /* No Execute Extension */
#define IA32_EFER_LMA         (1 << 10)       /* Long Mode Active */
#define IA32_EFER_LME         (1 << 8)        /* Long Mode Enable */
#define IA32_EFER_SCE         (1 << 0)        /* System Call Extension */

/* eax=0x00000001; ecx */
#define CPUID_SSE3            (1 << 0)        /* Streaming SIMD Extensions 3 (SSE3) */
#define CPUID_PCLMULQDQ       (1 << 1)        /* GF(2^k) Carry-less multiplication */
#define CPUID_DTES64          (1 << 2)        /* 64-bit DS Area */
#define CPUID_MONITOR         (1 << 3)        /* MONITOR/MWAIT instructions */
#define CPUID_DSCPL           (1 << 4)        /* CPL Qualified Debug Store */
#define CPUID_VMX             (1 << 5)        /* Virtual Machine Extensions */
#define CPUID_SMX             (1 << 6)        /* Safer Mode Exentions */
#define CPUID_EST             (1 << 7)        /* Enhanced SpeedStep Technology */
#define CPUID_TM2             (1 << 8)        /* Thermal Monitor 2 */
#define CPUID_SSSE3           (1 << 9)        /* Supplemental Streaming SIMD Extensions 3 */
#define CPUID_CNXT_ID         (1 << 10)       /* L1 Context ID */
#define CPUID_SDBG            (1 << 11)       /* Silicon Debug */
#define CPUID_FMA             (1 << 12)       /* Fused Multiply Add */
#define CPUID_CMPXCHG16B      (1 << 13)       /* CMPXCHG16B instruction */
#define CPUID_XTPR            (1 << 14)       /* xTPR Update Control */
#define CPUID_PDCM            (1 << 15)       /* Perfmon and Debug Capability */
#define CPUID_PCID            (1 << 17)       /* Process Context Identifiers */
#define CPUID_DCA             (1 << 18)       /* Direct cache access for DMA writes */
#define CPUID_SSE4_1          (1 << 19)       /* SSE4.1 instructions */
#define CPUID_SSE4_2          (1 << 20)       /* SSE4.2 instructions */
#define CPUID_X2APIC          (1 << 21)       /* x2APIC support */
#define CPUID_MOVBE           (1 << 22)       /* MOVBE instruction */
#define CPUID_POPCNT          (1 << 23)       /* POPCNT instruction */
#define CPUID_TSCDT           (1 << 24)       /* TSC Deadline Timer */
#define CPUID_AESNI           (1 << 25)       /* AESNI instructions */
#define CPUID_XSAVE           (1 << 26)       /* XSAVE/XRSTOR/XSETBV/XGETBV and XCR0 */
#define CPUID_OSXSAVE         (1 << 27)       /* CR4.OSXSAVE enabled to support XSAVE/XRSTOR */
#define CPUID_AVX             (1 << 28)       /* AVX instruction extensions */
#define CPUID_F16C            (1 << 29)       /* 16-bit floating-point conversion instructions */
#define CPUID_RDRND           (1 << 30)	      /* RDRAND instruction */
#define CPUID_HV              (1 << 31)       /* Running in Hypervisor */

/* eax=0x00000001; edx */
#define CPUID_FPU             (1 << 0)        /* x87 FPU */
#define CPUID_VME             (1 << 1)        /* Virtual 8086 mode extensions */
#define CPUID_DE              (1 << 2)        /* Debugging extensions (CR4.DE) */
#define CPUID_PSE             (1 << 3)        /* Page Size Extension */
#define CPUID_TSC             (1 << 4)        /* Time Stamp Counter */
#define CPUID_MSR             (1 << 5)        /* Model-specific Registers */
#define CPUID_PAE             (1 << 6)        /* Physical Address Extensions */
#define CPUID_MCE             (1 << 7)        /* Machine Check Excdeptions */
#define CPUID_CX8             (1 << 8)        /* Compare and Swap instructions */
#define CPUID_APIC            (1 << 9)        /* Advanced Programmable Interrupt Controller */
#define CPUID_SEP             (1 << 11)       /* SYSENTER and SYSEXIT instructions */
#define CPUID_MTRR            (1 << 12)       /* Memory Type Range Registers */
#define CPUID_PGE             (1 << 13)       /* Page Global bit (CR4.PGE) */
#define CPUID_MCA             (1 << 14)       /* Machine Check Architecture */
#define CPUID_CMOV            (1 << 15)       /* Conditional Move Instruction */
#define CPUID_PAT             (1 << 16)       /* Page Attribute Table */
#define CPUID_PSE36           (1 << 17)       /* 36-Bit Page Size Extension (4MiB pages) */
#define CPUID_PSN             (1 << 18)       /* Processor Serial Number */
#define CPUID_CLFLSH          (1 << 19)       /* CLFLUSH instruction */
#define CPUID_DS              (1 << 21)       /* Debug store: save trace of executed jumps */
#define CPUID_ACPI            (1 << 22)       /* Thermal Monitor and Clock MSRs */
#define CPUID_MMX             (1 << 23)       /* MMX instructions */
#define CPUID_FXSR            (1 << 24)       /* FXSAVE, FXRESTOR instructions (CR4.OSFXSR) */
#define CPUID_SSE             (1 << 25)       /* SSE instructions */
#define CPUID_SSE2            (1 << 26)       /* SSE2 instructions */
#define CPUID_SS              (1 << 27)       /* CPU cache supports self-snoop */
#define CPUID_HTT             (1 << 28)       /* Max APIC IDs reserved field is valid */
#define CPUID_TM              (1 << 29)       /* Thermal monitor limits temperature */
#define CPUID_PBE             (1 << 31)       /* Pending Break Enable */

/* eax=0x00000006; eax */
#define CPUID_ARAT            (1 << 2)        /* APIC-Timer-always-running feature */

/* eax=0x00000007; ebx */
#define CPUID_FSGSBASE        (1 << 0)        /* Access to FS and GS MSRs */
#define CPUID_TSC_ADJ         (1 << 1)        /* TSC adjust MSR */
#define CPUID_SGX             (1 << 2)        /* Sotware Guard Extensions */
#define CPUID_BMI1            (1 << 3)        /* Bit Manipulation Instruction Set 1 */
#define CPUID_HLE             (1 << 4)        /* Hardware Lock Elision */
#define CPUID_AVX2            (1 << 5)        /* Advanced Vector Extensions 2 */
#define CPUID_SMEP            (1 << 7)        /* Supervisor-Mode Execution Prevention */
#define CPUID_BMI2            (1 << 8)        /* Bit Manipulation Instruction Set 2 */
#define CPUID_ERMS            (1 << 9)        /* Enhanced REP MOVSB/STOSB */
#define CPUID_INVPCID         (1 << 10)       /* INVPCID instruction */
#define CPUID_RTM             (1 << 11)       /* Transactional Synchronization Extensions */
#define CPUID_PQM             (1 << 12)       /* Platform Quality of Service Monitoring */
#define CPUID_DFPUCSDS        (1 << 13)       /* Deprecates FPU CS and FPU DS */
#define CPUID_MPX             (1 << 14)       /* Memory Protection Extensions */
#define CPUID_PQE             (1 << 15)       /* Platform Quality of Service Enforcement */
#define CPUID_AVX512F         (1 << 16)       /* AVX512 Foundation */
#define CPUID_AVX512DQ        (1 << 17)       /* AVX512 Doubleword and Quadword Instructions */
#define CPUID_RDSEED          (1 << 18)       /* RDSEED instruction */
#define CPUID_ADX             (1 << 19)       /* Add-Carry Instruction Extensions */
#define CPUID_SMAP            (1 << 20)       /* Supervisor Mode Access Prevention */
#define CPUID_AVX512IMFA      (1 << 21)       /* AVX-512 Integer Fused Multiply-Add Instructions */
#define CPUID_PCOMMIT         (1 << 22)       /* PCOMMIT instruction */
#define CPUID_CLFLUSHOPT      (1 << 23)       /* CLFLUSHOPT instruction */
#define CPUID_CLWB            (1 << 24)       /* CLWB instruction */
#define CPUID_INTEL_PT        (1 << 25)       /* Intel Processor Tracec */
#define CPUID_AVX512PF        (1 << 26)       /* AVX-512 Prefetch Instructions */
#define CPUID_AVX512ER        (1 << 27)       /* AVX-512 Exponential and Reciprocal Instructions */
#define CPUID_AVX512CD        (1 << 28)       /* AVX-512 Conflict Detection Instructions */
#define CPUID_SHA             (1 << 29)       /* Intel SHA extensions */
#define CPUID_AVX512BW        (1 << 30)       /* AVX-512 Byte and Word Instructions */
#define CPUID_AVX512VL        (1 << 31)       /* AVX-512 Vector Length Extensions */

/* eax=0x00000007; ecx */
#define CPUID_PREFETCHWT1     (1 << 0)        /* PREFETCHWT1 instruction */
#define CPUID_AVX512VBMI      (1 << 1)        /* AVX-512 Vector Bit Manipulation Instructions */
#define CPUID_UMIP            (1 << 2)        /* User-mode Instruction Prevention */
#define CPUID_PKU             (1 << 3)        /* Memory Protection Keys for User-mode pages */
#define CPUID_OSPKE           (1 << 4)        /* PKU enabled by OS */
#define CPUID_AVX512VBMI2     (1 << 6)        /* AVX-512 Vector Bit Manipulation Instructions 2 */
#define CPUID_GFNI            (1 << 8)        /* Galois Field instructions */
#define CPUID_VAES            (1 << 9)        /* Vector AES instruction set (VEX-256/EVEX) */
#define CPUID_VPCLMULQDQ      (1 << 10)       /* CLMUL instruction set (VEX-256/EVEX) */
#define CPUID_AVX512VNNI      (1 << 11)       /* AVX-512 Vector Neural Network Instructions */
#define CPUID_AVX512BITALG    (1 << 12)       /* AVX-512 BITALG instructions */
#define CPUID_AVX512VPOPCNTDQ (1 << 14)       /* AVX-512 Vector POPCNt Doubleword and Quadword */
#define CPUID_MAWAU1          (1 << 17)       /* MPX Address-Width Adjust 1 */
#define CPUID_MAWAU2          (1 << 18)       /* MPX Address-Width Adjust 2 */
#define CPUID_MAWAU3          (1 << 19)       /* MPX Address-Width Adjust 3 */
#define CPUID_MAWAU4          (1 << 20)       /* MPX Address-Width Adjust 4 */
#define CPUID_MAWAU5          (1 << 21)       /* MPX Address-Width Adjust 5 */
#define CPUID_RDPID           (1 << 22)       /* Read Processor ID */
#define CPUID_SGX_LC          (1 << 30)       /* SGX Launch Configuration */

/* eax=0x00000007; edx */
#define CPUID_AVX512_4VNNIW   (1 << 2)        /* AVX-512 4-register Neural Network Instructions */
#define CPUID_AVX512_4FMAPS   (1 << 3)        /* AVX-512 4-register Multiply Accumulation Single precision */
#define CPUID_AVX512_PCONFIG  (1 << 18)       /* Platform configuration (Memory Encryption) */
#define CPUID_SPEC_CTRL_IBRS  (1 << 26)       /* Indirect Branch Speculation Control IBRS/IBPB */
#define CPUID_SPEC_CTRL_STIBP (1 << 27)       /* Single Thread Indirect Branch Predictor (STIBP) */

/* eax=0x80000001; ecx */
#define CPUID_LAHF_LM         (1 << 0)        /* LAHF/SAHF in long mode */
#define CPUID_LZCNT           (1 << 5)        /* LZCNT instruction */
#define CPUID_PREFETCHW       (1 << 8)        /* PREFETCHW instruction */

/* eax=0x80000001; edx */
#define CPUID_SYSCALL         (1 << 11)       /* SYSCALL and SYSRET instructions */
#define CPUID_NX              (1 << 20)       /* No Execute */
#define CPUID_PDPE1GB         (1 << 26)       /* Gigabyte pages */
#define CPUID_RDTSCP          (1 << 27)       /* RDTSCP instruction */
#define CPUID_LM              (1 << 29)       /* Long mode */

/* eax=0x80000007; edx */
#define CPUID_INVARIANT_TSC   (1 << 8)        /* Invariant TSC */

/* MSRs */
#define MSR_IA32_APIC_BASE                  0x0000001BU /* APIC base physical address */

/* X2APIC MSRs */
#define MSR_IA32_EXT_APIC_ID                0x00000802U	/* x2APIC ID */
#define MSR_IA32_EXT_APIC_VERSION           0x00000803U	/* x2APIC version */
#define MSR_IA32_EXT_APIC_TPR               0x00000808U /* x2APIC task priority */
#define MSR_IA32_EXT_APIC_PPR               0x0000080AU /* x2APIC processor priority */
#define MSR_IA32_EXT_APIC_EOI               0x0000080BU	/* x2APIC EOI */
#define MSR_IA32_EXT_APIC_LDR               0x0000080DU /* x2APIC logical destination */
#define MSR_IA32_EXT_APIC_SIVR              0x0000080FU /* x2APIC spurious interrupt vector */
#define MSR_IA32_EXT_APIC_ISR0              0x00000810U /* x2APIC in-service register 0 */
#define MSR_IA32_EXT_APIC_ISR1              0x00000811U /* x2APIC in-service register 1 */
#define MSR_IA32_EXT_APIC_ISR2              0x00000812U /* x2APIC in-service register 2 */
#define MSR_IA32_EXT_APIC_ISR3              0x00000813U /* x2APIC in-service register 3 */
#define MSR_IA32_EXT_APIC_ISR4              0x00000814U /* x2APIC in-service register 4 */
#define MSR_IA32_EXT_APIC_ISR5              0x00000815U /* x2APIC in-service register 5 */
#define MSR_IA32_EXT_APIC_ISR6              0x00000816U /* x2APIC in-service register 6 */
#define MSR_IA32_EXT_APIC_ISR7              0x00000817U /* x2APIC in-service register 7 */
#define MSR_IA32_EXT_APIC_TMR0              0x00000818U /* x2APIC trigger mode register 0 */
#define MSR_IA32_EXT_APIC_TMR1              0x00000819U /* x2APIC trigger mode register 1 */
#define MSR_IA32_EXT_APIC_TMR2              0x0000081AU /* x2APIC trigger mode register 2 */
#define MSR_IA32_EXT_APIC_TMR3              0x0000081BU /* x2APIC trigger mode register 3 */
#define MSR_IA32_EXT_APIC_TMR4              0x0000081CU /* x2APIC trigger mode register 4 */
#define MSR_IA32_EXT_APIC_TMR5              0x0000081DU /* x2APIC trigger mode register 5 */
#define MSR_IA32_EXT_APIC_TMR6              0x0000081EU /* x2APIC trigger mode register 6 */
#define MSR_IA32_EXT_APIC_TMR7              0x0000081FU /* x2APIC trigger mode register 7 */
#define MSR_IA32_EXT_APIC_IRR0              0x00000820U /* x2APIC interrupt request register 0 */
#define MSR_IA32_EXT_APIC_IRR1              0x00000821U /* x2APIC interrupt request register 1 */
#define MSR_IA32_EXT_APIC_IRR2              0x00000822U /* x2APIC interrupt request register 2 */
#define MSR_IA32_EXT_APIC_IRR3              0x00000823U /* x2APIC interrupt request register 3 */
#define MSR_IA32_EXT_APIC_IRR4              0x00000824U /* x2APIC interrupt request register 4 */
#define MSR_IA32_EXT_APIC_IRR5              0x00000825U /* x2APIC interrupt request register 5 */
#define MSR_IA32_EXT_APIC_IRR6              0x00000826U /* x2APIC interrupt request register 6 */
#define MSR_IA32_EXT_APIC_IRR7              0x00000827U /* x2APIC interrupt request register 7 */
#define MSR_IA32_EXT_APIC_ESR               0x00000828U /* x2APIC error status */
#define MSR_IA32_EXT_APIC_LVT_CMCI          0x0000082FU /* x2APIC LVT corrected machine check interrupt register */
#define MSR_IA32_EXT_APIC_ICR               0x00000830U /* x2APIC interrupt command register */
#define MSR_IA32_EXT_APIC_LVT_TIMER         0x00000832U /* x2APIC LVT timer interrupt register */
#define MSR_IA32_EXT_APIC_LVT_THERMAL       0x00000833U /* x2APIC LVT thermal sensor interrupt register */
#define MSR_IA32_EXT_APIC_LVT_PMI           0x00000834U /* x2APIC LVT performance monitor interrupt register */
#define MSR_IA32_EXT_APIC_LVT_LINT0         0x00000835U /* x2APIC LVT LINT0 register */
#define MSR_IA32_EXT_APIC_LVT_LINT1         0x00000836U /* x2APIC LVT LINT1 register */
#define MSR_IA32_EXT_APIC_LVT_ERROR         0x00000837U /* x2APIC LVT error register */
#define MSR_IA32_EXT_APIC_INIT_COUNT        0x00000838U /* x2APIC initial count register */
#define MSR_IA32_EXT_APIC_CUR_COUNT         0x00000839U /* x2APIC current count  register */
#define MSR_IA32_EXT_APIC_DIV_CONF          0x0000083EU /* x2APIC divide configuration register */
#define MSR_IA32_EXT_APIC_SELF_IPI          0x0000083FU /* x2APIC self IPI register */

#endif /* !_CPUDEF_H_ */
