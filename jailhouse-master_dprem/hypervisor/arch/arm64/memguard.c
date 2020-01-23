#include <asm/memguard.h>
#include <asm/sysregs.h>
#include <jailhouse/printk.h>
#include <asm/gic.h>
#include <asm/gic_v2.h>

#include <asm/percpu.h>

void call(int ms, int BW, int preemtion);

static void set_hyp_timer(u64 from, u64 interval)
{
	arm_write_sysreg(CNTHP_CVAL_EL2, from + interval);
	arm_write_sysreg(CNTHP_CTL_EL2, 1);
}

#define DEFAULT_EVENTS_MAX 10
#define DEBUG_MG

bool check_PMU_ISR(memguard_t *memguard)
{
	volatile u64 reg;

	arm_read_sysreg(PMOVSCLR_EL0, reg);
	return (reg & (1 << memguard->PMU_index));
}

static void set_PMU_counters(int index, int value)
{
	u64 reg;
	arm_read_sysreg(MDCR_EL2, reg);
	reg &= ~(0x1F);
	reg |= (1 << 7) + (index - 1);
	arm_write_sysreg(MDCR_EL2, reg);
	arm_write_sysreg(pmcntenclr_el0, 1 << index); //disable counters
	arm_write_sysreg(PMOVSCLR_EL0, 1 << index); //erase int flag
	arm_write_sysreg(PMSELR_EL0, index);
	arm_write_sysreg(PMXEVCNTR_EL0, (u64)(0xffffffff) - value);
	arm_write_sysreg(PMXEVTYPER_EL0, 0x19);
	arm_write_sysreg(PMINTENSET_EL1, 1 << index); //enable interrupt
	arm_write_sysreg(pmcntenset_el0, 1 << index); //enable counter
}

static void suspend(memguard_t *memguard)
{
	memguard->wait = 1;
	mmio_write32(memguard->gicc_base + GICC_PMR, 0xa0);
#ifdef DEBUG_MG
	printk("+");
#endif
	//set_PMU_counters(memguard->PMU_index, memguard->events_per_cycle);
	asm volatile("wfi" : : : "memory");
}

void memguard_init(memguard_t *memguard, void *gicd_base, void *gicc_base,
		   int cpu_id)
{
	u64 pmcr_reg;

	/* hypervisor timer */
	arm_read_sysreg(CNTFRQ_EL0,
			memguard->timer_periodic_increment); //frequency in HZ
	memguard->timer_periodic_increment /= 1000; //1ms
	arm_read_sysreg(CNTPCT_EL0, memguard->timer_last_value);
	/* read current time */
	set_hyp_timer(memguard->timer_last_value,
		      memguard->timer_periodic_increment);
	/* start periodic timer */
	memguard->events_per_cycle = DEFAULT_EVENTS_MAX;
	memguard->gicc_base = gicc_base;
	memguard->gicd_base = gicd_base;
	memguard->cpu_id = cpu_id;
	/*hyp timer enable*/
	mmio_write32(gicd_base + GICD_ISENABLER,(1 << 26));
	/*hyp timer priority setup*/
	u32 ipriority = mmio_read32(gicd_base + GICD_IPRIORITYR + (4 * 6));
	mmio_write32(gicd_base + GICD_IPRIORITYR + (4 * 6),
		     ((ipriority & (~(0xff << (8 * 2)))) | (0x80 << (8 * 2))));
	/*PMU priority setup*/
	mmio_write32(gicd_base + GICD_IPRIORITYR + (4 * 52), 0x90909090);
	/*PMU interrupt enable*/
	mmio_write32(gicd_base + GICD_ISENABLER + (6 * 4), 1 << (16 + cpu_id));
	/* PMU, use the last PMU counter*/
	arm_read_sysreg(PMCR_EL0, pmcr_reg);
	memguard->PMU_index = (((pmcr_reg & (0x1f << 11)) >> 11) - 1);
	set_PMU_counters(memguard->PMU_index, memguard->events_per_cycle);
	/* enable hyp timer int */
	mmio_write32(gicd_base + GICD_ISENABLER, (1 << 26));
}

void memguard_exit(memguard_t *memguard, void *gicd_base)
{
	/* disable hyp timer int */
	mmio_write32(gicd_base + GICD_ICENABLER, (1 << 26));
	/* disable counter */
	arm_write_sysreg(pmcntenclr_el0, 1 << (memguard->PMU_index));
	arm_write_sysreg(CNTHP_CTL_EL2, 0);
	arm_write_sysreg(MDCR_EL2, 0x6);
	memguard->wait = 0;
	memguard->suspended = 0;
	mmio_write32(memguard->gicc_base + GICC_PMR, 0xf0);
}

bool PMU_overflow_ISR(memguard_t *memguard, struct per_cpu *cpu_data)
{
	mmio_write32(memguard->gicc_base + GICC_EOIR, 208 + memguard->cpu_id);
	u64 reg;
	arm_read_sysreg(PMOVSCLR_EL0, reg);
	arm_write_sysreg(PMOVSCLR_EL0, (1 << (memguard->PMU_index)));
	cpu_data->stats[JAILHOUSE_CPU_STAT_VMEXITS_MANAGEMENT]++;
	mmio_write32(memguard->gicc_base + GICC_DIR, 208 + memguard->cpu_id);
	suspend(memguard);

	return true;
}

bool memguard_tick_ISR(memguard_t *memguard)
{
	mmio_write32(memguard->gicc_base + GICC_PMR, 0xf0);
	memguard->timer_last_value += memguard->timer_periodic_increment;
	set_hyp_timer(memguard->timer_last_value,
		      memguard->timer_periodic_increment);

	memguard->spinlock = 1;
	if (memguard->periods_left > 0) {
		set_PMU_counters(memguard->PMU_index,
				 memguard->events_per_cycle);
	} else {
		set_PMU_counters(memguard->PMU_index, DEFAULT_EVENTS_MAX);
	}

	memguard->periods_left--;
	memguard->spinlock = 0;

	memguard->suspended = 0;
	memguard->wait = 0;
	return true;
}

/**
 * Future syscall
 * preemption 1 = interrupts enable
 * preemption 0 = interrupts disable
 *
 * TODO possible race with interrupt enable?
 *
 * */
void MG_call(int periods, int events_per_period, int preemtion)
{
	struct per_cpu *cpu_data = this_cpu_data();
	while (cpu_data->memguard.spinlock)
		; /*spinlock caused by periods_left variable*/

	cpu_data->memguard.events_per_cycle = events_per_period;
	cpu_data->memguard.periods_left = periods;
	cpu_data->memguard.interrupt_enable = preemtion;

	memguard_tick_ISR(&(cpu_data->memguard));
}
