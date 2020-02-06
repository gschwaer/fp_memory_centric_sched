#include "ee_internal.h"

#include "uart-driver.h"
#include "printu.h"
#include "dprem.h"
#include "hardware_setup.h"
#include "memory_prefetching.h"
#include "system_config.h"

#include "benchmarking/average.h"
#include "benchmarking/sha.h"

#include <inmate.h>

#define CORE_0  0
#define CORE_1  1
#define CORE_2  2

#define TASK_1  0
#define TASK_2  1


#define A57_CORE_TO_BENCHMARK	CORE_0
#define TASK_TO_BENCHMARK		TASK_1

#define USE_TASK2  false


#define STR(x) ""#x
extern const uint8_t app_data[SIZE_DATA_SECTION];

OsEE_reg a57_core_id;

TaskType interfering_task_id;

TaskType task_activation_isr_id;

OsEE_reg task1_activation_period_ticks;
OsEE_reg task2_activation_period_ticks;
OsEE_reg task1_next_activation_ticks;
OsEE_reg task2_next_activation_ticks;
OsEE_reg task1_release_ticks;
OsEE_reg task2_release_ticks;
TaskType task1_id;
TaskType task2_id;
volatile bool task1_finished = true;
volatile bool task2_finished = true;
OsEE_reg task1_deadlines_missed = 0;
OsEE_reg task2_deadlines_missed = 0;
#define RELEASE_EVENT_APPLICATION_TASK1 0x04
#define RELEASE_EVENT_APPLICATION_TASK2 0x08

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

static inline OsEE_reg get_next_timer_ticks( OsEE_reg current_ticks )
{
	if(task1_next_activation_ticks < task2_next_activation_ticks) {
		return task1_next_activation_ticks - current_ticks;
	} else {
		return task2_next_activation_ticks - current_ticks;
	}
}

static inline void update_activation_ticks(OsEE_reg current_ticks, unsigned int task)
{
	if(task == TASK_1) {
		task1_next_activation_ticks = task1_next_activation_ticks + task1_activation_period_ticks;
	} else if(task == TASK_2) {
		task2_next_activation_ticks = task2_next_activation_ticks + task2_activation_period_ticks;
	}
}

static void task_activation_isr ( void ) {
	OsEE_reg current_ticks = osEE_aarch64_gtimer_get_ticks();

	if(current_ticks >= task1_next_activation_ticks) {
		// Run task 1
		update_activation_ticks(current_ticks, TASK_1);
		// check if deadline was missed
		if(task1_finished == false) {
			++task1_deadlines_missed;
			if(a57_core_id != A57_CORE_TO_BENCHMARK)
				printk("Erika-%lu: Task1 missed its deadline!\n", a57_core_id);
		} else {
			task1_deadlines_missed = 0;
			task1_finished = false;
			task1_release_ticks = current_ticks;
		}
		// Activate application tasks
		SetEvent(task1_id, RELEASE_EVENT_APPLICATION_TASK1);
	}
	if((current_ticks >= task2_next_activation_ticks) && (USE_TASK2 == true)) {
		// Run task 2
		update_activation_ticks(current_ticks, TASK_2);
		// check if deadline was missed
		if(task2_finished == false) {
			++task2_deadlines_missed;
			if(a57_core_id != A57_CORE_TO_BENCHMARK)
				printk("Erika-%lu: Task2 missed its deadline!\n", a57_core_id);
		} else {
			task2_deadlines_missed = 0;
			task2_finished = false;
			task2_release_ticks = current_ticks;
		}
		// Activate application tasks
		SetEvent(task2_id, RELEASE_EVENT_APPLICATION_TASK2);
	}

    // Restart timer
	if(USE_TASK2 == true)
		osEE_aarch64_gtimer_start(get_next_timer_ticks(current_ticks), OSEE_AARCH64_GTIMER_COUNTDOWN);
	else
		osEE_aarch64_gtimer_start(task1_next_activation_ticks - current_ticks, OSEE_AARCH64_GTIMER_COUNTDOWN);
}

static void application_task1_func( void )
{
	while(true) {
		// Wait for release
		WaitEvent(RELEASE_EVENT_APPLICATION_TASK1);
		ClearEvent(RELEASE_EVENT_APPLICATION_TASK1);

		// Memory Phase: begin
		OsEE_reg release_ts = DPREM_begin_memory_phase();

		// clear cache partition of this core and prefetch memory
		clear_cache_partition_L2(L2_FIRST_WAY, L2_LAST_WAY, PARTITION_FIRST_SET(a57_core_id), PARTITION_LAST_SET(a57_core_id));
		prefetch_memory((uint64_t)app_data, SIZE_DATA_SECTION/CACHE_LINE_SIZE);

		// Memory Phase: end (disabling interrupts)
		OsEE_reg int_flags = DPREM_end_memory_phase();
		OsEE_reg phase_change_ts = osEE_aarch64_gtimer_get_ticks();
		// Execution Phase: begin

		// execute benchmark
		benchmark_calculate_average();

		// Execution Phase: end (enabling interrupts)
		OsEE_reg finish_ts = osEE_aarch64_gtimer_get_ticks();

		// calculate response time
		if(a57_core_id == A57_CORE_TO_BENCHMARK && TASK_TO_BENCHMARK == TASK_1) {
			OsEE_reg block_time = osEE_aarch64_gtimer_ticks_to_ns(release_ts - task1_release_ticks);
			OsEE_reg mem_time = osEE_aarch64_gtimer_ticks_to_ns(phase_change_ts - release_ts);
			OsEE_reg exec_time = osEE_aarch64_gtimer_ticks_to_ns(finish_ts - phase_change_ts);
			printu("1,%lu,%lu,%lu,%lu\n",
					block_time,
					mem_time,
					exec_time,
					task1_deadlines_missed);
		}

		task1_finished = true;

		// reset priority ceiling after printing to not print interfering
		DPREM_end_execution_phase(int_flags);
	}
}

static void application_task2_func( void )
{
	if(!USE_TASK2) {
		printk("This task should not run!\n");
	}
	while(true) {
		// Wait for release
		WaitEvent(RELEASE_EVENT_APPLICATION_TASK2);
		ClearEvent(RELEASE_EVENT_APPLICATION_TASK2);

		// Memory Phase: begin
		OsEE_reg release_ts = DPREM_begin_memory_phase();

		// clear cache partition of this core and prefetch memory
		clear_cache_partition_L2(L2_FIRST_WAY, L2_LAST_WAY, PARTITION_FIRST_SET(a57_core_id), PARTITION_LAST_SET(a57_core_id));
		prefetch_memory((uint64_t)app_data, SIZE_DATA_SECTION/CACHE_LINE_SIZE);

		// Memory Phase: end (disabling interrupts)
		OsEE_reg int_flags = DPREM_end_memory_phase();
		OsEE_reg phase_change_ts = osEE_aarch64_gtimer_get_ticks();
		// Execution Phase: begin

		// execute benchmark
		benchmark_calculate_sha();
		//benchmark_calculate_average();

		// Execution Phase: end (enabling interrupts)
		OsEE_reg finish_ts = osEE_aarch64_gtimer_get_ticks();

		// calculate response time
		if(a57_core_id == A57_CORE_TO_BENCHMARK && TASK_TO_BENCHMARK == TASK_2) {
			OsEE_reg block_time = osEE_aarch64_gtimer_ticks_to_ns(release_ts - task2_release_ticks);
			OsEE_reg mem_time = osEE_aarch64_gtimer_ticks_to_ns(phase_change_ts - release_ts);
			OsEE_reg exec_time = osEE_aarch64_gtimer_ticks_to_ns(finish_ts - phase_change_ts);
			printu("2,%lu,%lu,%lu,%lu\n",
					block_time,
					mem_time,
					exec_time,
					task2_deadlines_missed);
		}

		task2_finished = true;

		// reset priority ceiling after printing to not print interfering
		DPREM_end_execution_phase(int_flags);
	}
}

#define CORE_0_BASE_FREQ  100
#define CORE_1_BASE_FREQ   99
#define CORE_2_BASE_FREQ   98
#define BASE_FREQ_MULT      3
#define NOT_BENCH_MULT      2
static inline OsEE_reg get_task_frequency(unsigned int task)
{
	OsEE_reg task_freq = -1; // provoke deadline miss if core id is wrong
	if(a57_core_id == 0) { // high prio core
		if(task == TASK_1)
			task_freq = CORE_0_BASE_FREQ*BASE_FREQ_MULT;
		else
			task_freq = CORE_0_BASE_FREQ;
	} else if(a57_core_id == 1) { // mid prio core
		if(task == TASK_1)
			task_freq = CORE_1_BASE_FREQ*BASE_FREQ_MULT;
		else
			task_freq = CORE_1_BASE_FREQ;
	} else if(a57_core_id == 2) {  // low prio core
		if(task == TASK_1)
			task_freq = CORE_2_BASE_FREQ*BASE_FREQ_MULT;
		else
			task_freq = CORE_2_BASE_FREQ;
	}

	if(A57_CORE_TO_BENCHMARK == a57_core_id)
		return task_freq;
	else
		return task_freq * NOT_BENCH_MULT;
}

int main(void){
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

	if(a57_core_id <= A57_CORE_TO_BENCHMARK) {
		// Create benchmarking tasks

		//  Activation ISR
		CreateTask(&task_activation_isr_id, OSEE_TASK_TYPE_ISR2, task_activation_isr, 1U, 1U, 1U, OSEE_SYSTEM_STACK);
		SetISR2Source(task_activation_isr_id, OSEE_GTIMER_IRQ);

		//  Application tasks
		//   Average benchmark
		TaskPrio task1_prio = 2;
		MemSize task1_stack_size = 1024*4;
		OsEE_reg task1_activation_frequency_hz = get_task_frequency(TASK_1);
		CreateTask(&task1_id, OSEE_TASK_TYPE_EXTENDED, application_task1_func, task1_prio, task1_prio, 1U, task1_stack_size);

		//   SHA benchmark
		TaskPrio task2_prio = 1;
		MemSize task2_stack_size = 1024*8;
		OsEE_reg task2_activation_frequency_hz = get_task_frequency(TASK_2);
		if(USE_TASK2) {
			CreateTask(&task2_id, OSEE_TASK_TYPE_EXTENDED, application_task2_func, task2_prio, task2_prio, 1U, task2_stack_size);
		}

		printk("Erika-%lu: task_freq:", a57_core_id);
		printk(" %lu", task1_activation_frequency_hz);
		if(USE_TASK2)
			printk(" %lu", task2_activation_frequency_hz);
		printk(".\n");

		// Set up activation periods
		uint64_t ticks_per_second = osEE_aarch64_gtimer_get_freq();
		task1_activation_period_ticks = ticks_per_second / task1_activation_frequency_hz;
		task2_activation_period_ticks = ticks_per_second / task2_activation_frequency_hz;

		// Setup the UART
		if(a57_core_id == A57_CORE_TO_BENCHMARK) {
			if(UART_init(UART_C, UART_BAUD_115200) == 0) {
				printk("Erika-%lu: UART initialization failed!\n", a57_core_id);
			}
		}
	} else {
		// Create interfering task

		TaskPrio task_prio = 1;
		MemSize task_stack_size = 1024;
		CreateTask(&interfering_task_id, OSEE_TASK_TYPE_EXTENDED, interfering_task_func, task_prio, task_prio, 1U, task_stack_size);
	}

    // Start scheduler
    StartOS(OSDEFAULTAPPMODE);

    if(a57_core_id <= A57_CORE_TO_BENCHMARK) {
    	// Start application tasks (they will block and wait for activation)
    	ActivateTask(task1_id);
    	if(USE_TASK2)
    		ActivateTask(task2_id);

		if(a57_core_id == A57_CORE_TO_BENCHMARK) {
			printk("Erika-%lu: Benchmarking on A57-core %lu, task %u.\n", a57_core_id, a57_core_id, TASK_TO_BENCHMARK+1);
		} else {
			printk("Erika-%lu: Application interference from A57-core %lu.\n", a57_core_id, a57_core_id);
		}

		OsEE_reg current_ticks = osEE_aarch64_gtimer_get_ticks();

		// Reset time keeping
		task1_next_activation_ticks = current_ticks;
		task2_next_activation_ticks = current_ticks;
		update_activation_ticks(current_ticks, TASK_1);
		update_activation_ticks(current_ticks, TASK_2);

		// Set the activation timer
		osEE_aarch64_gtimer_start(get_next_timer_ticks(current_ticks), OSEE_AARCH64_GTIMER_COUNTDOWN);
    } else {
    	// Start interfering task
		ActivateTask(interfering_task_id);

		printk("Erika-%lu: Worst case interference from A57-core %lu.\n", a57_core_id, a57_core_id);
    }

    /* endless loop*/
    while(1) {
        __asm__ volatile("wfi": : : "memory");
    }
    return 0;
}
