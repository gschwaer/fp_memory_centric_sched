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
	struct jailhouse_memory mem_regions[9];
	struct jailhouse_memory_colored col_mem[1];
	struct jailhouse_irqchip irqchips[1];
	struct jailhouse_pci_device pci_devices[6];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "jetson-tx2-MM-ee4-col",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_memory_regions_colored = ARRAY_SIZE(config.col_mem),
		.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
		.vpci_irq_base = 321,

	},

	.cpus = {
		0x20, /* CPU 5 */
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
		/* IVHSMEM  7 (LINUX -> EE4) */ {
                        .phys_start = 0x2750c0000,
                        .virt_start = 0x2750c0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,

                },
                /* IVHSMEM  8 (LINUX <- EE4) */ {
                        .phys_start = 0x2750e0000,
                        .virt_start = 0x2750e0000,
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
		 /* IVHSMEM  16 (EE2 -> E4) */ {
                        .phys_start = 0x2751e0000,
                        .virt_start = 0x2751e0000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                         | JAILHOUSE_MEM_ROOTSHARED,
                },
				/* IVHSMEM  17 (EE2 <- E4) */ {
						.phys_start = 0x275200000,
						.virt_start = 0x275200000,
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

			 // Assign 8 of 8 avail. colors
			.colors=0xff000000,
		},
	},

	.irqchips = {
		/* GIC */
		{
			.address = 0x03881000,
			.pin_base = 288,
			.pin_bitmap = {
				0,
				0,
				0x1f << (353 - 352) /* irq 353 -> 357 */
			},
		},
	},

	.pci_devices = {
		/* LINUX -> EE4 0000.00.4*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4,
                        .bar_mask = {
                                0xffffff00, 0xffffffff, 0x00000000,
                                0x00000000, 0x00000000, 0x00000000,
                        },

                        /*num_msix_vectors needs to be 0 for INTx operation*/
                        .num_msix_vectors = 0,
                        .shmem_region = 2,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* LINUX <- EE4 0000.04.0*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4 << 3,
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
                        .shmem_region = 4,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
                        .domain = 0x0,
                },
		/* EE1 <- EE4 0000.15.0 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x15 << 3,
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
		/* EE2 > EE4 0000.02.4 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x4 | (2 << 3),
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
		/* EE4 -> EE2 0000.03.4 */
		 {
				 .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
				 .bdf = 0x4 | (0x3 << 3),
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
	 },
};
