#include "ee_internal.h"

#include "uart-driver.h"
#include "printu.h"
#include "dprem.h"
#include "hardware_setup.h"
#include "memory_prefetching.h"
#include "system_config.h"

#include "benchmarking/average.h"

#include <inmate.h>


#define A57_CORE_TO_BENCHMARK 2


extern const uint8_t app_data[SIZE_DATA_SECTION];

OsEE_reg a57_core_id;

TaskType interfering_task_id;

OsEE_reg activation_ticks;
TaskType task_activation_isr_id;

TaskType application_task_id1;
bool application_task1_finished = true;
#define RELEASE_EVENT_APPLICATION_TASK1 0x04

static void interfering_task_func( void )
{
	while(true) {
		// Memory Phase: begin
		DPREM_begin_memory_phase();

		// clear cache partition of this core and prefetch memory
		clear_cache_partition_L2(L2_FIRST_WAY, L2_LAST_WAY, PARTITION_FIRST_SET(a57_core_id), PARTITION_LAST_SET(a57_core_id));
		prefetch_memory((uint64_t)app_data, SIZE_DATA_SECTION/CACHE_LINE_SIZE);

		// Memory Phase: end (disabling interrupts)
		OsEE_reg int_flags = DPREM_end_memory_phase();

		// Execution Phase: empty
		DPREM_end_execution_phase(int_flags);
	}
}

static void task_activation_isr ( void ) {
	// Activate application tasks
    SetEvent(application_task_id1, RELEASE_EVENT_APPLICATION_TASK1);
    // check if deadline was missed
    if(application_task1_finished == false) {
    	printk("Erika-%lu:Task1 missed its deadline!\n", a57_core_id);
    }

    // Restart timer
    osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);
}

static void application_task1_func( void )
{
	while(true) {
		// Wait for release
		WaitEvent(RELEASE_EVENT_APPLICATION_TASK1);
		ClearEvent(RELEASE_EVENT_APPLICATION_TASK1);
		OsEE_reg release_ts = osEE_aarch64_gtimer_get_ticks();

		// Memory Phase: begin
		DPREM_begin_memory_phase();

		// clear cache partition of this core and prefetch memory
		clear_cache_partition_L2(L2_FIRST_WAY, L2_LAST_WAY, PARTITION_FIRST_SET(a57_core_id), PARTITION_LAST_SET(a57_core_id));
		prefetch_memory((uint64_t)app_data, SIZE_DATA_SECTION/CACHE_LINE_SIZE);

		// Memory Phase: end (disabling interrupts)
		OsEE_reg int_flags = DPREM_end_memory_phase();
		OsEE_reg phase_change_ts = osEE_aarch64_gtimer_get_ticks();
		// Execution Phase: begin

		// execute benchmark
		task_calculate_average();

		// Execution Phase: end (enabling interrupts)
		OsEE_reg finish_ts = osEE_aarch64_gtimer_get_ticks();
		DPREM_end_execution_phase(int_flags);

		// calculate response time
		if(a57_core_id == A57_CORE_TO_BENCHMARK) {
			printu("%lu,%lu,%lu\n",
					osEE_aarch64_gtimer_ticks_to_ns(release_ts),
					osEE_aarch64_gtimer_ticks_to_ns(phase_change_ts),
					osEE_aarch64_gtimer_ticks_to_ns(finish_ts));
		}
	}
}

int main(void){
	OsEE_reg activation_frequency_hz;

	// setup hardware
	disable_predictor();
	disable_prefetcher();

	// figure out A57 core id
	// the used cache partition depends on this
	// it should return values 0-3 depending on the core Erika is run on
	OsEE_reg reg;
	OSEE_AARCH64_MRS(reg, MPIDR_EL1);
	a57_core_id = (reg & 0x3);

	// Initialize hypervisor based memory arbitration
	DPREM_init();

	if(a57_core_id == A57_CORE_TO_BENCHMARK) {
		// Create benchmarking tasks

		//  Activation ISR
		activation_frequency_hz = 100;
		CreateTask(&task_activation_isr_id, OSEE_TASK_TYPE_ISR2, task_activation_isr, 1U, 1U, 1U, OSEE_SYSTEM_STACK);
		SetISR2Source(task_activation_isr_id, OSEE_GTIMER_IRQ);

		//  Application tasks
		TaskPrio task_prio = 1;
		MemSize task_stack_size = 1024;
		CreateTask(&application_task_id1, OSEE_TASK_TYPE_EXTENDED, application_task1_func, task_prio, task_prio, 1U, task_stack_size);

		// Setup the UART
		if(UART_init(UART_C, UART_BAUD_115200) == 0) {
			printk("Erika-%lu: UART initialization failed!\n", a57_core_id);
		}
	} else {
		// Create interfering task

		TaskPrio task_prio = 1;
		MemSize task_stack_size = 1024;
		CreateTask(&interfering_task_id, OSEE_TASK_TYPE_EXTENDED, interfering_task_func, task_prio, task_prio, 1U, task_stack_size);
	}

    // Start scheduler
    StartOS(OSDEFAULTAPPMODE);

    if(a57_core_id == A57_CORE_TO_BENCHMARK) {
    	// Start benchmarking tasks

		// Application tasks (they will block and wait for activation)
		ActivateTask(application_task_id1);

		// Set the activation timer
		uint64_t ticks_per_second = osEE_aarch64_gtimer_get_freq();
		activation_ticks = ticks_per_second / activation_frequency_hz;
		osEE_aarch64_gtimer_start(activation_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);

		printk("Erika-%lu: Benchmarking on A57-core %u with activation frequency %lu Hz.\n", a57_core_id, a57_core_id, activation_frequency_hz);
    } else {
    	// Start tasks (they will block and wait for activation)
		ActivateTask(interfering_task_id);

		printk("Erika-%lu: Interfering from A57-core %u.\n", a57_core_id, a57_core_id);
    }

    /* endless loop*/
    while(1) {
        __asm__ volatile("wfi": : : "memory");
    }
    return 0;
}
