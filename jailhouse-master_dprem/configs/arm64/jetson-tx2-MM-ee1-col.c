/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (C) 2018 Evidence Srl
 *
 * Authors:
 *  Luca Cuomo <l.cuomo@evidence.eu.com>
 *
 * This is confidential code meant only for usage by Magneti Marelli
 * in the context of the HERCULES H2020 European project.
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[12];
	struct jailhouse_memory_colored col_mem[1];
	struct jailhouse_irqchip irqchips[1];
	struct jailhouse_pci_device pci_devices[8];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "jetson-tx2-MM-ee1-col",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_memory_regions_colored = ARRAY_SIZE(config.col_mem),
		.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
		.vpci_irq_base = 300,

	},

	.cpus = {
		0x1,
	},

	.mem_regions = {
		/* UART */ {
			.phys_start = 0x3100000,
			.virt_start = 0x3100000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* communication region */ {
			.virt_start = 0x80000000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_COMM_REGION,
		},
		/* FAT-FS SHMEM*/ {
                        .phys_start = 0x274800000,
                        .virt_start = 0x274800000,
                        .size = 0x800000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,

                },
		/* IVHSMEM  1 (LINUX -> EE1) */ {
                        .phys_start = 0x275000000,
                        .virt_start = 0x275000000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,
                },
                /* IVHSMEM  2 (LINUX <- E1) */ {
                        .phys_start = 0x275020000,
                        .virt_start = 0x275020000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                         | JAILHOUSE_MEM_ROOTSHARED,
                },
		/* IVHSMEM  9 (EE1 -> EE2) */ {
                        .phys_start = 0x275100000,
                        .virt_start = 0x275100000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,
                },
                /* IVHSMEM  10 (EE1 <- E2) */ {
                        .phys_start = 0x275120000,
                        .virt_start = 0x275120000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                         | JAILHOUSE_MEM_ROOTSHARED,
                },
		/* IVHSMEM  11 (EE1 -> EE3) */ {
                        .phys_start = 0x275140000,
                        .virt_start = 0x275140000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,
                },
                /* IVHSMEM  12 (EE1 <- E3) */ {
                        .phys_start = 0x275160000,
                        .virt_start = 0x275160000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                         | JAILHOUSE_MEM_ROOTSHARED,
                },
		/* IVHSMEM  13 (EE1 -> EE4) */ {
                        .phys_start = 0x275180000,
                        .virt_start = 0x275180000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,
                },
                /* IVHSMEM  14 (EE1 <- E4) */ {
                        .phys_start = 0x2751a0000,
                        .virt_start = 0x2751a0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                         | JAILHOUSE_MEM_ROOTSHARED,
                },
				/* PROXY PTP OVER SHMEM AREA*/ {
		    	.phys_start = 0x27501f000,
		        .virt_start = 0x27501f000,
		       	.size = 0x1000,
		       	.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
						JAILHOUSE_MEM_ROOTSHARED,
		       	},
	},

	.col_mem = {
		{
			/* RAM */
			.memory = {
				.phys_start = 0x1bca00000,
				.virt_start = 0,
				// The real size is
				// size * 1/4 = 49 MB
				.size = 0xc400000,
				.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
					JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
			},

			 // Assign 8 of 32 avail. colors
			.colors=0x000000ff,
		},
	},

	.irqchips = {
		/* GIC */
		{
			.address = 0x03881000,
			.pin_base = 288,
			.pin_bitmap = {
				0,
				0x3ff << (332 - 320) /* irq 332 -> 341 */
			},
		},
	},

	.pci_devices = {
		/* LINUX -> EE1 0000.00.1*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x1,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 3,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* LINUX <- EE1 0000.01.0*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x1 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 4,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,
                },
		/* EE1 -> EE2 0000.01.2 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x2 | (1 << 3),
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 5,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* EE1 <- EE2 0000.12.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x12 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 6,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,
                },
		/* EE1 -> EE3 0000.01.3 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3 | (0x1 << 3),
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 7,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* EE1 <- EE3 0000.13.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x13 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 8,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,
                },
		/* EE1 -> EE4 0000.01.4 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4 | (1 << 3),
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 9,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* EE1 <- EE4 0000.15.0, this should be 0000.14.0 but
		the pin rotation of IRQ assignment in IVSHMEM is done
		on 3 bit of the device paramter of the PCI. We must
		have the mask 001 in order to obtain the last available
		IRQ which does not overlap with others*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x15 << 3,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 10,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,
                },
        },
};
