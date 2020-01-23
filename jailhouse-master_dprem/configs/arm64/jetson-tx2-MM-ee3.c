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
	struct jailhouse_irqchip irqchips[1];
	struct jailhouse_pci_device pci_devices[5];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "jetson-tx2-MM-ee3",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
		.vpci_irq_base = 316,

	},

	.cpus = {
		0x10, /* CPU 4 */
	},

	.mem_regions = {
		/* UART */ {
			.phys_start = 0x3100000,
			.virt_start = 0x3100000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* RAM */ {
			.phys_start = 0x26BF00000,
			.virt_start = 0,
			.size = 0x2000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* communication region */ {
			.virt_start = 0x80000000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_COMM_REGION,
		},
		/* IVHSMEM  5 (LINUX -> EE3) */ {
                        .phys_start = 0x275080000,
                        .virt_start = 0x275080000,
                        .size = 0x10000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE
                        | JAILHOUSE_MEM_ROOTSHARED,

                },
                /* IVHSMEM  6 (LINUX <- EE3) */ {
                        .phys_start = 0x2750a0000,
                        .virt_start = 0x2750a0000,
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
		/* IVHSMEM  15 (EE2 <- EE3) */ {
                        .phys_start = 0x2751c0000,
                        .virt_start = 0x2751c0000,
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

	.irqchips = {
		/* GIC */
		{
			.address = 0x03881000,
			.pin_base = 288,
			.pin_bitmap = {
				0,
				0xf << (348 - 320) /* irq 348 -> 352 */,
				1  << (353 - 352) /* irq 353 */
			},
		},
	},

	.pci_devices = {
		/* LINUX -> EE3 0000.00.3*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3,
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
		/* LINUX <- EE3 0000.03.0*/
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3 << 3,
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
		/* EE1 -> EE3 0000.01.3 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3 | (1 << 3),
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
                        .shmem_region = 6,
                        .shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
			.domain = 0x0,
                },
		/* EE2 <- EE3 0000.02.3 */
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x3 | (0x2 << 3),
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
