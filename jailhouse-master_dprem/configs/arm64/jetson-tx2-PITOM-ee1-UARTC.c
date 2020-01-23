/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Configuration for gic-demo or uart-demo inmate on Nvidia Jetson TX2:
 * 1 CPU, 64 MB RAM, serial port 0
 *
 * This is confidential code meant only for usage by Pitom
 * in the context of the HERCULES H2020 European project.
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[6];
	struct jailhouse_irqchip irqchips[2];
	struct jailhouse_pci_device pci_devices[2];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "jetson-tx2-PITOM-ee1",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
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
			.size = 0x20000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO,
		},
		/* RAM */ {
			.phys_start = 0x270000000,
			.virt_start = 0,
			.size = 0x1000000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* communication region */ {
			.virt_start = 0x80000000,
			.size = 0x00001000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_COMM_REGION,
		},

		/* IVHSMEM  1*/ {
                        .phys_start = 0x275000000,
                        .virt_start = 0x275000000,
                        .size = 0x100000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE 
                        | JAILHOUSE_MEM_ROOTSHARED,

                },

                /* IVHSMEM  2*/ {
                        .phys_start = 0x275200000,
                        .virt_start = 0x275200000,
                        .size = 0x100000,
                        .flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE 
                         | JAILHOUSE_MEM_ROOTSHARED, 
                },
		/* UART C */ {
			.phys_start = 0x0c280000,
			.virt_start = 0x0c280000,
			.size = 0x10000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO,
		},
	},

	.irqchips = { 
		/* GIC */
		{ /* UART setting B -> F */
			.address = 0x03881000,
			.pin_base = 32,
			.pin_bitmap = {
				0,
				0,
				0,
				0x1f << (113 - 96) /* irq 113 -> 117 */
			},
		}, 
		/* GIC */
		{
			.address = 0x03881000,
			.pin_base = 288,
			.pin_bitmap = {
				0,
				3 << (332 - 320) /* irq 332 and 333 */
			},
		},
	},

	.pci_devices = {
                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0x0 << 3,
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

                {
                        .type = JAILHOUSE_PCI_TYPE_IVSHMEM,
                        .bdf = 0xf << 3,
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
        },

};
