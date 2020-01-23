/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 * Copyright (c) Siemens AG, 2014-2017
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <mach.h>
#include <inmate.h>

#define BEATS_PER_SEC		1

//static u64 ticks_per_beat;
//static volatile u64 expected_ticks;


static void handle_IRQ(unsigned int irqn)
{
//	static u64 min_delta = ~0ULL, max_delta = 0;
	static int cnt = 0;
//	u64 delta;

	if (irqn == 15)	{
		if (cnt%2)
			printk("irq: enter memory\n");
		else
			printk("irq: leave memory\n");

		cnt++;
	}
#if 0
	if (irqn==TIMER_IRQ) {
		cnt++;
		expected_ticks = timer_get_ticks();
		timer_start(ticks_per_beat);
		if (cnt%2)
			jailhouse_call_arg1(0xFF,5);
		else
			jailhouse_call(0xFE);
		return;
	}
#endif

//	printk("!!!\n");


//	if (irqn != TIMER_IRQ)
//		return;
#if 0
	delta = timer_get_ticks() - expected_ticks;
	if (delta < min_delta)
		min_delta = delta;
	if (delta > max_delta)
		max_delta = delta;

	printk("%6ld\n",(long)timer_ticks_to_ns(delta));
#endif

#if 0
	printk("Timer fired, jitter: %6ld ns, min: %6ld ns, max: %6ld ns\n",
	       (long)timer_ticks_to_ns(delta),
	       (long)timer_ticks_to_ns(min_delta),
	       (long)timer_ticks_to_ns(max_delta));
#endif

//	expected_ticks = timer_get_ticks() + ticks_per_beat;
//	timer_start(ticks_per_beat);
}

void inmate_main(void)
{
//	int ret;

	printk("Initializing the GIC...\n");
	gic_setup(handle_IRQ);
//	gic_enable_irq(TIMER_IRQ);
	gic_enable_irq(15);


	jailhouse_call_arg1(0xFF,5);

//	printk("returned value from hypercall: 0x%d\n",ret);
//	ret = jailhouse_call_arg1(0xFF,2);
//	printk("returned value from hypercall: 0x%d\n",ret);

//	printk("Initializing the timer...\n\n");
//	ticks_per_beat = timer_get_frequency() / BEATS_PER_SEC;
//	expected_ticks = timer_get_ticks();
//	timer_start(ticks_per_beat);




	halt();
}
