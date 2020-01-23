/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <inmate.h>
#define GICD_SGIR	0x0f00	

void send_ipi(int, int);



void send_ipi(int ipi_number, int dest_cpu)
{
       	u32 *gicd_base = (u32 *) 0x03881000;
	//mmio_write32(gicd_base + (GICD_SGIR / 4), (dest_cpu << 16) | ipi_number);
	mmio_write32(gicd_base + GICD_SGIR, (dest_cpu << 16) | ipi_number);
}


void inmate_main(void)
{
	unsigned int i = 0, j;
	/*
	 * The cell config can set up a mapping to access UARTx instead of UART0
	 */

	printk("Hello %d from cell!\n", i);
	while(++i) {
		for (j = 0; j < 100000000; j++);
		send_ipi(15,0);
		send_ipi(15,1);
		send_ipi(15,2);
		send_ipi(15,3);
//		heartbeat();
	}

	/* lr should be 0, so a return will go back to the reset vector */
}
