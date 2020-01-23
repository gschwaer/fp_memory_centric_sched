/* Request memory with priority 20 with frequency BEATS_PER_SEC
 * It uses a hypercall that disables the memory request at the end of the experiment 
 * Used with arb_overhead_1 to measure the overhad of memory arbitratrion  
 */

#include <mach.h>
#include <inmate.h>

#define BEATS_PER_SEC		10

static u64 ticks_per_beat;


static void handle_IRQ(unsigned int irqn)
{
	 if (irqn != TIMER_IRQ)
                return

	printk("Calling hypercall\n");
	(void) jailhouse_call_arg1(0xFD,20);
	timer_start(ticks_per_beat);
}



void inmate_main(void)
{
	gic_setup(handle_IRQ);
	gic_enable_irq(TIMER_IRQ);

	ticks_per_beat = timer_get_frequency() / BEATS_PER_SEC;
	timer_start(ticks_per_beat);

	halt();
}
