/*
 * HERCULES IRQ response time test
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 * This code is based on gic-demo.c
 */

#include <mach.h>
#include <inmate.h>
#include <caches.h>
#include <sleep.h>

#define BEATS_PER_SEC		32
// Tunables
static u64 SAMPLES=100;
//#define DEBUG_OUTPUT

static u64 ticks_per_beat;
static volatile u64 expected_ticks;
static u64 counter = 0;

static void handle_IRQ(unsigned int irqn)
{
	static u64 min_delta = ~0ULL, max_delta = 0;
	u64 delta;

	if (irqn != TIMER_IRQ || counter == SAMPLES)
		return;

	delta = timer_get_ticks() - expected_ticks;
	if (delta < min_delta)
		min_delta = delta;
	if (delta > max_delta)
		max_delta = delta;

#ifdef DEBUG_OUTPUT
	printk("Timer fired, jitter: %6ld ns, min: %6ld ns, max: %6ld ns\n",
	       (long)timer_ticks_to_ns(delta),
	       (long)timer_ticks_to_ns(min_delta),
	       (long)timer_ticks_to_ns(max_delta));
#else
	printk("%llu\n",(long)timer_ticks_to_ns(delta));
#endif
	counter++;
	//flush_i_cache();
	expected_ticks = timer_get_ticks() + ticks_per_beat;
	timer_start(ticks_per_beat);
}

void inmate_main(void)
{
	sleep(5);
	printk("Initializing the GIC...\n");
	gic_setup(handle_IRQ);
	gic_enable_irq(TIMER_IRQ);

	printk("Initializing the timer...\n");
	ticks_per_beat = timer_get_frequency() / BEATS_PER_SEC;
	expected_ticks = timer_get_ticks() + ticks_per_beat;
	timer_start(ticks_per_beat);

	halt();
}
