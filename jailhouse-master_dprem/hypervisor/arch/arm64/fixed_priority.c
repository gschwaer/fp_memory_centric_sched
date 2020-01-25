#ifdef CONFIG_JAILHOUSE_FIXED_PRIORITY

#include "asm/fixed_priority.h"

#include <jailhouse/control.h>
#include <jailhouse/hypercall.h>
#include <jailhouse/printk.h>
#include <asm/memory_arbitration.h>

#include <asm/irqchip.h>
#include <asm/spinlock.h>

// Priority is based on MAX_PRIORITY-cell_id (higher cell id -> lower priority)
#define MAX_PRIORITY		6

//#define SUSPEND_LINUX
#define TOKEN_FREE		0xFF
#define TOKEN_NULL_PRIORITY	0x00

static DEFINE_SPINLOCK(memory_lock);
static volatile unsigned int memory_requests[6] = { TOKEN_NULL_PRIORITY };
static volatile unsigned int token_owner        = TOKEN_FREE;
static volatile unsigned int token_priority     = TOKEN_NULL_PRIORITY;


unsigned long timer_get_frequency(void);
static unsigned long emul_division(u64, u64);
u64 timer_ticks_to_ns(u64);

unsigned long timer_get_frequency(void)
{
        unsigned long freq;

        arm_read_sysreg(CNTFRQ_EL0, freq);
        return freq;
}

static unsigned long emul_division(u64 val, u64 div)
{
        unsigned long cnt = 0;

        while (val > div) {
                val -= div;
                cnt++;
        }
        return cnt;
}

u64 timer_ticks_to_ns(u64 ticks)
{
        return emul_division(ticks * 1000,
                             timer_get_frequency() / 1000 / 1000);
}


static int request_memory(struct per_cpu *cpu_data)
{
	unsigned int cpu_id = cpu_data->cpu_id;
	unsigned int priority = MAX_PRIORITY - cpu_data->cell->config->id;

	spin_lock(&memory_lock);

#ifdef SUSPEND_LINUX
	bool token_was_free = (token_owner == TOKEN_FREE);
#endif

	memory_requests[cpu_id] = priority;
	if (priority > token_priority) {
		if (token_owner != TOKEN_FREE && token_owner != cpu_id) {
			//printk("Send interrupt to reclaim the token from %d and give to %d\n", token_owner, cpu_id);
			irqchip_set_pending(per_cpu(token_owner),JAILHOUSE_MEMORY_ARBITRATION_SGI_MEMORY_PHASE_ENDED);
		}
		token_owner    = cpu_id;
		token_priority = priority;
	}

#ifdef SUSPEND_LINUX
	// suspend Linux if token was acquired
	bool token_is_free = (token_owner == TOKEN_FREE);
	if(token_was_free == true && token_is_free == false)
	{
		//printk("Suspending Linux\n");
		unsigned int cpu;
		for_each_cpu(cpu, root_cell.cpu_set) {
			arch_suspend_cpu(cpu);
		}
	}
#endif

	int got_token = (token_owner == cpu_id);

	spin_unlock(&memory_lock);

	return got_token; 
}

static int disable_memory_request(unsigned int cpu_id)
{
	struct cell *cell;

	spin_lock(&memory_lock);

	memory_requests[cpu_id] = TOKEN_NULL_PRIORITY;

	if(token_owner == cpu_id)
	{
		token_owner    = TOKEN_FREE;
		token_priority = TOKEN_NULL_PRIORITY;

		for_each_cell(cell) {
			unsigned int cpu_id_it = first_cpu(cell->cpu_set);
			if (memory_requests[cpu_id_it] > token_priority) {
				token_priority = memory_requests[cpu_id_it];
				token_owner    = cpu_id_it;
			}
		}

		//printk("New token holder: %d\nNew token priority: %d\n",token_owner,token_priority);

		if (token_owner != TOKEN_FREE) {
			//printk("Send interrupt to pass the token from %d to %d\n", cpu_id, token_owner);
			irqchip_set_pending(per_cpu(token_owner),JAILHOUSE_MEMORY_ARBITRATION_SGI_MEMORY_PHASE_STARTED);
		}

#ifdef SUSPEND_LINUX
		// resume Linux if token was released
		bool token_is_free = (token_owner == TOKEN_FREE);
		if(token_is_free == true)
		{
			//printk("Resuming Linux\n");
			unsigned int cpu;
			for_each_cpu(cpu, root_cell.cpu_set) {
				arch_resume_cpu(cpu);
			}
		}
#endif
	}

	spin_unlock(&memory_lock);

	return 0;
}

/* Measures the overhead need for arbitratration excluding the hypercall overhead 
 * Two cell should be running: cell1 and cell2.
 * cell1 requests memory only once and uses the  standard request_memory function with priority prio1
 * cell2 calls the function enable_memory_request_test_arbitration_overhead many times with prio2 > prio1
 * cell2 does not call any other function, enable_memory_request_test_arbitration_overhead disables the request at the end of the experiment
 */
static int enable_memory_request_test_arbitration_overhead(struct per_cpu *cpu_data, unsigned int priority)
{
	unsigned int cpu_id = cpu_data->cpu_id;
	u64 start = 0, end = 0;


	arm_read_sysreg(CNTHP_TVAL_EL2, start);

	spin_lock(&memory_lock);

	memory_requests[cpu_id] = priority;
	if (priority > token_priority) {
		if (token_owner != TOKEN_FREE)
			irqchip_set_pending(per_cpu(token_owner),10);
		token_owner    = cpu_id;
		token_priority = priority;
	}

	spin_unlock(&memory_lock);

	
	arm_read_sysreg(CNTHP_TVAL_EL2, end);

	
	printk("Hypervisor overhead: %lld ns\n", timer_ticks_to_ns(start - end));


	disable_memory_request(cpu_data->cpu_id);


	return (token_owner == cpu_id);
}

// called from OS via hvc
long fixed_priority_hypercall(struct per_cpu *cpu_data, unsigned long action_code)
{
	switch(action_code) {

		case JAILHOUSE_MEMORY_ARBITRATION_BEGIN_MEM_PHASE:
		{
			int res = request_memory(cpu_data);
			if(res == true) {
				return JAILHOUSE_MEMORY_ARBITRATION_ACK;
			} else {
				return JAILHOUSE_MEMORY_ARBITRATION_NACK;
			}
		}
		break;

		case JAILHOUSE_MEMORY_ARBITRATION_END_MEM_PHASE:
		{
			disable_memory_request(cpu_data->cpu_id);
			return JAILHOUSE_MEMORY_ARBITRATION_ACK;
		}
		break;

		case 0xFFFF:
		{
			// currently not working / not tested
			enable_memory_request_test_arbitration_overhead(cpu_data, 0);
			return JAILHOUSE_MEMORY_ARBITRATION_ACK;
		}
		break;

		default:
		{
			printk("TDMA: Core %u send invalid TDMA action code %lu!\n", cpu_data->cpu_id, action_code);
			return JAILHOUSE_MEMORY_ARBITRATION_INVALID;
		}
	}
}

void on_arch_park_cpu(unsigned int cpu_id)
{
	struct cell *cell = per_cpu(cpu_id)->cell;

	printk("FixedPriority: Resetting token on CPU %u for cell \"%s\".\n",
		cpu_id,
		cell->config->name);
	
	disable_memory_request(cpu_id);
}

#endif /* CONFIG_JAILHOUSE_FIXED_PRIORITY */
