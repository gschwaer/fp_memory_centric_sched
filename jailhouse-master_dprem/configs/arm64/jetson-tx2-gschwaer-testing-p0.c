/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Configuration for gic-demo or uart-demo inmate on Nvidia Jetson TX2:
 * 1 CPU, 64 MB RAM, serial port 0
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[3];
	struct jailhouse_memory_colored col_mem[1];
	struct jailhouse_irqchip irqchips[2];
	struct jailhouse_pci_device pci_devices[0];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "gschwaer-testing-col-p0",
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
		/* UART-A Extension Header */ {
			.phys_start = 0x03100000,
			.virt_start = 0x03100000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				 JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
		/* RAM * {
			.phys_start = 0x0000000270000000 +0x10000,
			.virt_start = 0,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				 JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* Test Data Region * {
			.phys_start = 0x0000000270100000,
			.virt_start = 0x100000,
			.size = 0x200000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE,
		},
		/* communication region */ {
			.virt_start = 0x80000000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				 JAILHOUSE_MEM_COMM_REGION,
		},
		/* UART-C Serial Header */ {
			.phys_start = 0x0c280000,
			.virt_start = 0x0c280000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				 JAILHOUSE_MEM_IO | JAILHOUSE_MEM_ROOTSHARED,
		},
	},

	.col_mem = {
		{
			/* RAM */
			.memory = {
				.phys_start = 0x270000000,
				.virt_start = 0,
				.size = 0x400000,
				.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
					JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
			},

			/* Assigning quarter of the colors */
			.colors=0x000000ff,
		},
	},

	.irqchips = {
		/* GIC */
		{ /* UART setting C */
			.address = 0x03881000,
			.pin_base = 32,
			.pin_bitmap = {
				0,
				0,
				0,
				1 << (114 - 96) /* irq 114 */
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
};
