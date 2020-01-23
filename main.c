#include "ee_internal.h"

#include "uart-driver.h"
#include "printu.h"
#include "dprem.h"
#include "hardware_setup.h"
#include "memory_prefetching.h"
#include "system_config.h"

#include "sorting.h"

#include <inmate.h>

extern uint64_t size_memory_application_task1;
extern void *memory_application_task1; // max size: SIZE_DATA_SECTION

OsEE_reg activation_ticks;
TaskType task_activation_isr_id;
TaskType application_task_id1;

#define RELEASE_EVENT_APPLICATION_TASK1 0x04

static void task_activation_isr ( void ) {
	// Activate application tasks
    SetEvent(application_task_id1, RELEASE_EVENT_APPLICATION_TASK1);

    // Restart timer
    osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);
}

static void application_task1( void )
{
	while(true) {
		// Wait for activation
		WaitEvent(RELEASE_EVENT_APPLICATION_TASK1);
		ClearEvent(RELEASE_EVENT_APPLICATION_TASK1);

		OsEE_reg release_ts = osEE_aarch64_gtimer_get_ticks();

		// Memory Phase: load data
		DPREM_begin_memory_phase();

		// clear and prefetch memory
		clear_cache_partition_L2(L2_FIRST_WAY, L2_LAST_WAY, PARTITION_FIRST_SET, PARTITION_LAST_SET);
		prefetch_memory((uint64_t)memory_application_task1, size_memory_application_task1/CACHE_LINE_SIZE);

		DPREM_end_memory_phase();
		OsEE_reg phase_change_ts = osEE_aarch64_gtimer_get_ticks();

		// Execution Phase: execute task
		sorting_task_main();

		OsEE_reg finish_ts = osEE_aarch64_gtimer_get_ticks();

		// calculate response time
		printu("<App1:%lu,%lu,%lu>\n",
				osEE_aarch64_gtimer_ticks_to_ns(release_ts),
				osEE_aarch64_gtimer_ticks_to_ns(phase_change_ts),
				osEE_aarch64_gtimer_ticks_to_ns(finish_ts));
	}
}

int main(void){
	// setup hardware
	disable_predictor();
	disable_prefetcher();

	// Initialize hypervisor based memory arbitration
	DPREM_init();

	// Create activation task
	OsEE_reg activation_frequency_hz = 20;
    CreateTask(&task_activation_isr_id, OSEE_TASK_TYPE_ISR2, task_activation_isr, 1U, 1U, 1U, OSEE_SYSTEM_STACK);
    SetISR2Source(task_activation_isr_id, OSEE_GTIMER_IRQ);

    // Create application tasks
    TaskPrio task_prio = 1;
    MemSize task_stack_size = 1024;
    CreateTask(&application_task_id1, OSEE_TASK_TYPE_EXTENDED, application_task1, task_prio, task_prio, 1U, task_stack_size);

    // Setup the UART
    if(UART_init(UART_C, UART_BAUD_115200) == 0)
    	printk("Erika: UART initialization failed!\n");

    // Start scheduler
    StartOS(OSDEFAULTAPPMODE);

    // Start tasks (they will block and wait for activation)
    ActivateTask(application_task_id1);

    // Set the activation timer
    uint64_t ticks_per_second = osEE_aarch64_gtimer_get_freq();
    activation_ticks = ticks_per_second / activation_frequency_hz;
    printk("Erika: Starting with activation frequency %lu Hz.\n", activation_frequency_hz);
    osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);

    /* endless loop*/
    while(1) {
        __asm__ volatile("wfi": : : "memory");
    }
    return 0;
}
