#include <mach.h>
#include <inmate.h>


void inmate_main(void)
{
	u64 start, end;

	while(1) {
		// flush instruction cache
		asm volatile ("ic iallu");
		asm volatile ("dsb sy" : : : "memory");
		asm volatile ("dsb ish" : : : "memory");
		// measure the hypercall overhead
		start = timer_get_ticks();
		jailhouse_call_arg1(0xFC, 1);
		end = timer_get_ticks();
		printk("%6ld\n",(long)timer_ticks_to_ns(end-start));
	}
	halt();
}
