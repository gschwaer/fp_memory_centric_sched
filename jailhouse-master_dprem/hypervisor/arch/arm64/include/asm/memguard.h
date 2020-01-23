#ifndef _JAILHOUSE_ASM_MEMGUARD_H
#define _JAILHOUSE_ASM_MEMGUARD_H

#include <jailhouse/types.h>

#define HYPERVISOR_TIMER 26
#define PMU_INT 23


typedef struct{
	u64 timer_periodic_increment;
	u64 timer_last_value;
	u32 events_per_cycle;
	u32 periods_left;
	u32 interrupt_enable;
	u32 suspended;
	u32 wait;
	u32 PMU_index;
	u32 spinlock;
	void * gicc_base;
	void * gicd_base;
	u32 cpu_id;
}memguard_t;

#include <asm/percpu.h>

void memguard_init(memguard_t * memguard, void*, void*,int);
void memguard_exit(memguard_t * memguard, void*);
bool PMU_overflow_ISR(memguard_t * memguard,struct per_cpu *cpu_data);
bool memguard_tick_ISR(memguard_t * memguard);
bool check_PMU_ISR(memguard_t * memguard);
void MG_call(int ms, int BW, int preemtion);


#endif
