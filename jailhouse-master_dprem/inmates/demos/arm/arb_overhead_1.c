/* Request memory with priority 5 and do nothing 
 * Used with arb_overhead_2 to measure the overhad of memory arbitratrio  
 */

#include <mach.h>
#include <inmate.h>


void inmate_main(void)
{
	jailhouse_call_arg1(0xFF,5);
	halt();
}
