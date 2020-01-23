/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Configuration for Jailhouse Jetson TX2 board
 *
 * Copyright (C) 2018 Evidence Srl
 *
 * Authors:
 *  Claudio Scordino <claudio@evidence.eu.com>
 *  Luca Cuomo <l.cuomo@evidence.eu.com>
 *
 * This is confidential code meant only for usage by Magneti Marelli
 * in the context of the HERCULES H2020 European project.
 *
 * NOTE: Add "mem=7790M vmalloc=512M" to the kernel command line.
 *
 *	2:6900:0000 inmate (size: 2 * 200:0000 + 2 * 310:0000 = 162 MB)
 *	2:7100:0000 hypervisor (size: 380:0000 = 56 MB)
 *
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

struct {
	struct jailhouse_system header;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[67];
	struct jailhouse_irqchip irqchips[3];
	struct jailhouse_pci_device pci_devices[8];
} __attribute__((packed)) config = {
	.header = {
		.signature = JAILHOUSE_SYSTEM_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.hypervisor_memory = {
			.phys_start = 0x271000000,
			.size = 0x3800000,
		},
		.debug_console = {
			.address = 0x3100000,
			.size = 0x10000,
			.flags = JAILHOUSE_CON1_TYPE_8250 |
				 JAILHOUSE_CON1_ACCESS_MMIO |
				 JAILHOUSE_CON1_REGDIST_4 |
				 JAILHOUSE_CON2_TYPE_ROOTPAGE,
		},
		.platform_info = {
			/* .pci_mmconfig_base is fixed; if you change it,
                         update the value in inmates/lib/arm-common/pci.c
                         (PCI_CFG_BASE) and regenerate the inmate library*/
                        .pci_mmconfig_base = 0x40000000,
                        .pci_mmconfig_end_bus = 0x0,
                        .pci_is_virtual = 1,

			.arm = {
				.gicd_base = 0x03881000,
				.gicc_base = 0x03882000,
				.gich_base = 0x03884000,
				.gicv_base = 0x03886000,
				.gic_version = 2,
				.maintenance_irq = 25,
			}
		},
		.root_cell = {
			.name = "Jetson-TX2-MM-root",
			.cpu_set_size = sizeof(config.cpus),
			.num_memory_regions = ARRAY_SIZE(config.mem_regions),
			.num_pci_devices = ARRAY_SIZE(config.pci_devices),
			.num_irqchips = ARRAY_SIZE(config.irqchips),
			.vpci_irq_base = 288,
		},
	},

	.cpus = {
		/* Start Hyp on all core (include Denver CPUs [1,2]) */
		0x3f /* All the cores */,
	},


	.mem_regions = {
		/* BPMP_ATCM */ {
                        .phys_start = 0x00000000,
                        .virt_start = 0x00000000,
                        .size = 0x40000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },

		/* MISC */ {
                        .phys_start = 0x00100000,
                        .virt_start = 0x00100000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },

		/* AXIP2P */ {
			.phys_start = 0x02100000,
			.virt_start = 0x02100000,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* GPIO_CTL */ {
			.phys_start = 0x02200000,
			.virt_start = 0x02200000,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* AXI2APB */ {
			.phys_start = 0x02300000,
			.virt_start = 0x02300000,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},

		/* TSA */ {
			.phys_start = 0x2400000,
			.virt_start = 0x2400000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* PADCTL_A (PINMUX) */ {
			.phys_start = 0x02430000,
			.virt_start = 0x02430000,
			.size = 0x15000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* UFSHC */ {
			.phys_start = 0x02450000,
			.virt_start = 0x02450000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* ETHER_QOS */ {
			.phys_start = 0x02490000,
			.virt_start = 0x02490000,
			.size = 0x50000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* GPCDMA */ {
			.phys_start = 0x02600000,
			.virt_start = 0x02600000,
			.size = 0x210000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* APE */ {
			.phys_start = 0x02900000,
			.virt_start = 0x02900000,
			.size = 0x200000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* MSS */ {
			.phys_start = 0x02c00000,
			.virt_start = 0x02c00000,
			.size = 0xb0000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* LIC */ {
                        .phys_start = 0x03000000,
                        .virt_start = 0x03000000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* TOP_TKE */ {
			.phys_start = 0x03010000,
			.virt_start = 0x03010000,
			.size = 0xe0000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* TIMER */ {
                        .phys_start = 0x03020000,
                        .virt_start = 0x03020000,
                        .size = 0xa0000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* UARTA */ {
			.phys_start = 0x03100000,
			.virt_start = 0x03100000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				     JAILHOUSE_MEM_EXECUTE,
		},
		/* UART-B */ {
			.phys_start = 0x03110000,
			.virt_start = 0x03110000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* I2C */ {
			.phys_start = 0x03160000,
			.virt_start = 0x03160000,
			.size = 0x90000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* PWM1 + PWM2 */ {
			.phys_start = 0x03280000,
			.virt_start = 0x03280000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* PWM3 - PWM8 */ {
			.phys_start = 0x032a0000,
			.virt_start = 0x032a0000,
			.size = 0x60000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SDMMC */ {
			.phys_start = 0x3400000,
			.virt_start = 0x3400000,
			.size = 0x80000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SATA */ {
			.phys_start = 0x3500000,
			.virt_start = 0x3500000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* HDA */ {
			.phys_start = 0x3510000,
			.virt_start = 0x3510000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* XUSB_PADCTL + XUSB_HOST */ {
			.phys_start = 0x3520000,
			.virt_start = 0x3520000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* XUSB */ {
			.phys_start = 0x03540000,
			.virt_start = 0x03540000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* FUSE + KFUSE */ {
			.phys_start = 0x03820000,
			.virt_start = 0x03820000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* MIPICAL */ {
			.phys_start = 0x03990000,
			.virt_start = 0x03990000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* TACH_0 */ {
			.phys_start = 0x039c0000,
			.virt_start = 0x039c0000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SE0 */ {
			.phys_start = 0x03ac0000,
			.virt_start = 0x03ac0000,
			.size = 0x30000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* TOP0_HSP */{
                        .phys_start = 0x03c00000,
                        .virt_start = 0x03c00000,
                        .size = 0xa0000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* VIC CAR */{
                        .phys_start = 0x05560000,
                        .virt_start = 0x05560000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* CSITE */ {
			.phys_start = 0x08000000,
			.virt_start = 0x08000000,
			.size = 0x2000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SCE VIC registers */ {
			.phys_start = 0x0b020000,
			.virt_start = 0x0b020000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SCE_PM */ {
			.phys_start = 0x0b1f0000,
			.virt_start = 0x0b1f0000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SCE_CFG */ {
			.phys_start = 0x0b230000,
			.virt_start = 0x0b230000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* AON Cluster */ {
			.phys_start = 0x0c000000,
			.virt_start = 0x0c000000,
			.size = 0x800000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* BPMP VIC registers */ {
			.phys_start = 0x0d020000,
			.virt_start = 0x0d020000,
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* ACTMON  + SIMON + SOC_THERM */ {
			.phys_start = 0x0d230000,
			.virt_start = 0x0d230000,
			.size = 0x70000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/*CCPLEX CLUSTER*/{
                        .phys_start = 0x0e000000,
                        .virt_start = 0x0e000000,
                        .size = 0x400000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* PCIE0 */ {
			.phys_start = 0x10000000,
			.virt_start = 0x10000000,
			.size = 0x1000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SMMU0 */ {
			.phys_start = 0x12000000,
			.virt_start = 0x12000000,
			.size = 0x1000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* HOST1X */ {
			.phys_start = 0x13e00000,
			.virt_start = 0x13e00000,
			.size = 0x90000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* HOST1X_ACTMON */ {
			.phys_start = 0x13ec0000,
			.virt_start = 0x13ec0000,
			.size = 0x50000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* DPAUX1 */ {
			.phys_start = 0x15040000,
			.virt_start = 0x15040000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* NVCSI */ {
			.phys_start = 0x150c0000,
			.virt_start = 0x150c0000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* TSECB */ {
			.phys_start = 0x15100000,
			.virt_start = 0x15100000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* NVDISPLAY */ {
                        .phys_start = 0x15200000,
                        .virt_start = 0x15200000,
                        .size = 0x40000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },
		/* VIC */ {
			.phys_start = 0x15340000,
			.virt_start = 0x15340000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* NVJPG */ {
			.phys_start = 0x15380000,
			.virt_start = 0x15380000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* NVDEC + NVENC + TSEC + ISP  + SOR */ {
			.phys_start = 0x15480000,
			.virt_start = 0x15480000,
			.size = 0x1c0000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* NI */ {
			.phys_start = 0x15700000,
			.virt_start = 0x15700000,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SE1-SE4 */ {
			.phys_start = 0x15810000,
			.virt_start = 0x15810000,
			.size = 0x40000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* GPU */ {
			.phys_start = 0x17000000,
			.virt_start = 0x17000000,
			.size = 0x9000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* SYSRAM_0 */{
                        .phys_start = 0x30000000,
                        .virt_start = 0x30000000,
                        .size = 0x10000000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
                                JAILHOUSE_MEM_EXECUTE,
                },

		/* System RAM */ {
			.phys_start = 0x80000000,
			.virt_start = 0x80000000,
			.size = 0x1E6E00000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* Inmate (2 * 32 MB + 2 * 49 MB) */ {
			.phys_start = 0x266E00000,
			.virt_start = 0x266E00000,
			.size = 0xA200000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
		/* FAT-FS SHMEM*/ {
                        .phys_start = 0x274800000,
                        .virt_start = 0x274800000,
                        .size = 0x800000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,

                },
		/* IVHSMEM  1 (LINUX -> EE1)*/ {
                        .phys_start = 0x275000000,
                        .virt_start = 0x275000000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,

                },
		/* IVHSMEM  2 (LINUX <- EE1)*/ {
                        .phys_start = 0x275020000,
                        .virt_start = 0x275020000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE ,

                },
		/* IVHSMEM  3 (LINUX -> EE2)*/ {
                        .phys_start = 0x275040000,
                        .virt_start = 0x275040000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* IVHSMEM  4 (LINUX <- EE2)*/ {
                        .phys_start = 0x275060000,
                        .virt_start = 0x275060000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* IVHSMEM  5 (LINUX -> EE3)*/ {
                        .phys_start = 0x275080000,
                        .virt_start = 0x275080000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* IVHSMEM  6 (LINUX <- EE3)*/ {
                        .phys_start = 0x2750a0000,
                        .virt_start = 0x2750a0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* IVHSMEM  7 (LINUX -> EE4)*/ {
                        .phys_start = 0x2750c0000,
                        .virt_start = 0x2750c0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* IVHSMEM  8 (LINUX <- EE4)*/ {
                        .phys_start = 0x2750e0000,
                        .virt_start = 0x2750e0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE ,

                },
		/* PROXY PTP OVER SHMEM AREA*/ {
                        .phys_start = 0x27501f000,
                        .virt_start = 0x27501f000,
                        .size = 0x1000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_ROOTSHARED,
                },
		/* Persistent RAM */ {
			.phys_start = 0x277080000,
			.virt_start = 0x277080000,
			.size = 0x200000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE,
		},
	},
	.irqchips = {
		/* GIC */ {
			.address = 0x03881000,
			.pin_base = 32,
			.pin_bitmap = {
				0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
			},
		},
		/* GIC */ {
			.address = 0x03881000,
			.pin_base = 160,
			.pin_bitmap = {
				0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
			},
		},
		/* GIC */ {
			.address = 0x03881000,
			.pin_base = 288,
			.pin_bitmap = {
				0xffffffff, 0xffffffff, 0xffffffff
			},
		},
	},

	.pci_devices = {
		/* PCI LINUX -> EE1 Bdf 0000.00.1 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x1 ,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 57,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,

                },
		/* PCI LINUX <- EE1 Bdf 0000.01.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x1 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 58,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,

                },
		/* PCI LINUX -> EE2 Bdf 0000.00.2 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x2,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 59,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,

                },
		/* PCI LINUX <- EE2 Bdf 0000.02.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x2 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 60,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,

                },
		/* PCI LINUX -> EE3 Bdf 0000.00.3 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 61,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,

                },
		/* PCI LINUX <- EE3 Bdf 0000.03.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 62,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,

                },
		/* PCI LINUX -> EE4 Bdf 0000.00.4 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 63,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,

                },
		/* PCI LINUX <- EE4 Bdf 0000.04.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 64,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,

                },
        },
};
