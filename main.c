#include "ee_internal.h"
#include "uart-driver.h"
#include "printu.h"
#include "dprem.h"
#include <inmate.h>

OsEE_reg activation_ticks;
TaskType task_activation_isr_id;
TaskType application_task_id1;

#define RELEASE_EVENT_APPLICATION_TASK1 0x04

static void task_activation_isr ( void ) {
	printu(".");

	// Activate application tasks
    SetEvent(application_task_id1, RELEASE_EVENT_APPLICATION_TASK1);

    // Restart timer
    osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);
    return;
}

static void application_task1( void )
{
	volatile uint64_t long_running_task_counter = 0;
	while(true) {
		// Wait for activation
		WaitEvent(RELEASE_EVENT_APPLICATION_TASK1);
		ClearEvent(RELEASE_EVENT_APPLICATION_TASK1);

		printu("<");

		DPREM_begin_memory_phase();

		// Memory Phase: load data

		// this only delays
		printu("m(s");
		long_running_task_counter = 0;
		while(long_running_task_counter < 1000000) {
			++long_running_task_counter;
		}
		printu("e)");

		DPREM_end_memory_phase();

		// Execution Phase: execute task

		// this only delays
		printu(",c");
		long_running_task_counter = 0;
		while(long_running_task_counter < 1000000) {
			++long_running_task_counter;
		}

		printu(">");
	}
}

int main(void){
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
    printk("Erika: Starting timer with freq=%lu Hz.\n", activation_frequency_hz);
    osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);

    /* endless loop*/
    while(1) {
        __asm__ volatile("wfi": : : "memory");
    }
    return 0;
}
