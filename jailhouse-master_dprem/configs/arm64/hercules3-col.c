/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Hercules configuration for testing - CORE 3 [COLORED]
 *
 * Copyright (C) 2018, Università di Modena e Reggio Emilia
 *
 * Authors:
 *    Luca Miccio <lucmiccio@gmail.com>
 *
 * This configuration is intended to be used with the corresponding
 * hercules_root configuration and only for testing the HERCULES
 * inmates.
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
	struct jailhouse_memory mem_regions[2];
	struct jailhouse_memory_colored col_mem[1];
	struct jailhouse_irqchip irqchips[1];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "HERCULES_3_COL",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_memory_regions_colored = ARRAY_SIZE(config.col_mem),
		.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_pci_devices = 0,
		.vpci_irq_base = 300,

	},

	.cpus = {
		0x8, // Core 3
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

	},

	.col_mem = {
		{
			/* RAM */
			.memory = {
				.phys_start = 0x200000000,
				.virt_start = 0,
				.size = 0x8000000,
				.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
					JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
			},

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
				0x1 << (329 - 320) /* PMU IRQ */
			},
		},
	},

};
