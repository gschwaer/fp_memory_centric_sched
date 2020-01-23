/*
 * TDMA for Jailhouse based on code from "Memguard for Jailhouse".
 *
 * TDMA for Jailhouse:
 *  Copyright (c) Technical University of Munich, 2020
 *
 * Memguard for Jailhouse:
 *  Copyright (c) Czech Technical University in Prague, 2018
 *
 * Authors Memguard for Jailhouse:
 *  Joel Matějka <matejjoe@fel.cvut.cz>
 *  Michal Sojka <michal.sojka@cvut.cz>
 *  Přemysl Houdek <houdepre@fel.cvut.cz>
 *
 * Authors TDMA for Jailhouse:
 *  Gero Schwäricke <gero.schwaericke@tum.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#ifdef CONFIG_JAILHOUSE_TDMA

#include <asm/tdma.h>

#include <jailhouse/printk.h>
#include <jailhouse/control.h>
#include <jailhouse/hypercall.h>

#include <asm/sysregs.h>
#include <asm/irqchip.h>
#include <asm/gic.h>
#include <asm/gic_v2.h>
#include <asm/percpu.h>
#include <asm/memory_arbitration.h>

#define MS_TO_US( ms )		(( ms ) * 1000ULL )
#define S_TO_US( s )		(( s ) * 1000000ULL )

/**
 *  Config for TDMA module
 *  Slots are assigned by cell id (cell 1 has the first slot, cell 3 the last if
 *  tdma_num_slots == 3).
 *  PMU (Blocking a core on memory oversusage is currently not enabled
 *  (commented out), because it is not well tested.)
 */
static const u8 tdma_num_slots = 3;
static const u64 tdma_slot_period_us = S_TO_US(2);
//#define PMU_BUDGET 10


/* Found out by using Linux perf tool and watching /proc/interrupts
 * Parker manual says:
 *   The total size of 384 corresponds to:
 *   32 first IDs are SGI and PPI
 *   288 next IDs are global SPI, one to one mapped to the 288 LIC interrupts
 *   64 next IDs are local SPI, generated inside CCPLEX and for CCPLEX use only
 */
#define CCPLEX_IRQ_SIZE			384

/* Conversion from cpu_id to PMU IRQ number
 *
 * Number 296 is defined in device tree which corresponds to:
 * (32 SGI and PPI +) 288 global SPI + 4 local SPI
 * This number is base for A57 cluster, 320 is for Denvers
 */
static const int tx2_cpu_id2irqn[6] = {
	32 + 296,
	32 + 320,
	32 + 321,
	32 + 297,
	32 + 298,
	32 + 299,
};

/* On Parker, only 16 priority levels are implemented */
#define IRQ_PRIORITY_MIN	0xF0
#define IRQ_PRIORITY_MAX	0x00
#define IRQ_PRIORITY_INC	0x10

#define IRQ_PRIORITY_THR	0x10

/* Address of a bit for e.g. enabling of irq with id m is calculated as:
 * ADDR = BASE + (4 * n) where n = m / 32
 * Then the position of the bit in the register is calculated as:
 * POS = m MOD 32 */
#define IRQ_BIT_OFFSET(x)	(4 * ((x)/32))
#define IRQ_BIT_POSITION(x)	((x) % 32)

/* Similarly for bytes (e.g. irq priority) */
#define IRQ_BYTE_OFFSET(x)	(4 * ((x)/4))
#define IRQ_BYTE_POSITION(x)	(((x) % 4) * 8)
#define IRQ_BYTE_MASK		0xFF

#define CNTHP_CTL_EL2_ENABLE	(1<<0)
#define CNTHP_CTL_EL2_IMASK	(1<<1)

#if !defined(UINT32_MAX)
#define UINT32_MAX		0xffffffffU /* 4294967295U */
#endif

#if !defined(UINT64_MAX)
#define UINT64_MAX		0xffffffffffffffffULL /* 18446744073709551615 */
#endif

/* Reg def copied from kvm_arm.h */
/* Hyp Debug Configuration Register bits */
#define MDCR_EL2_TDRA		(1 << 11)
#define MDCR_EL2_TDOSA		(1 << 10)
#define MDCR_EL2_TDA		(1 << 9)
#define MDCR_EL2_TDE		(1 << 8)
#define MDCR_EL2_HPME		(1 << 7)
#define MDCR_EL2_TPM		(1 << 6)
#define MDCR_EL2_TPMCR		(1 << 5)
#define MDCR_EL2_HPMN_MASK	(0x1F)

#define PMCR_EL0_N_POS		(11)
#define PMCR_EL0_N_MASK		(0x1F << PMCR_EL0_N_POS)

#define PMEVTYPER_P		(1 << 31) /* EL1 modes filtering bit */
#define PMEVTYPER_U		(1 << 30) /* EL0 filtering bit */
#define PMEVTYPER_NSK		(1 << 29) /* Non-secure EL1 (kernel) modes filtering bit */
#define PMEVTYPER_NSU		(1 << 28) /* Non-secure User mode filtering bit */
#define PMEVTYPER_NSH		(1 << 27) /* Non-secure Hyp modes filtering bit */
#define PMEVTYPER_M		(1 << 26) /* Secure EL3 filtering bit */
#define PMEVTYPER_MT		(1 << 25) /* Multithreading */
#define PMEVTYPER_EVTCOUNT_MASK	0x3ff

/* PMU events copied from drivers/misc/tegra-profiler/armv8_events.h */

/* Required events. */
#define QUADD_ARMV8_HW_EVENT_PMNC_SW_INCR		0x00
#define QUADD_ARMV8_HW_EVENT_L1_DCACHE_REFILL		0x03
#define QUADD_ARMV8_HW_EVENT_L1_DCACHE_ACCESS		0x04
#define QUADD_ARMV8_HW_EVENT_PC_BRANCH_MIS_PRED		0x10
#define QUADD_ARMV8_HW_EVENT_CLOCK_CYCLES		0x11
#define QUADD_ARMV8_HW_EVENT_PC_BRANCH_PRED		0x12

/* At least one of the following is required. */
#define QUADD_ARMV8_HW_EVENT_INSTR_EXECUTED		0x08
#define QUADD_ARMV8_HW_EVENT_OP_SPEC			0x1B

/* Common architectural events. */
#define QUADD_ARMV8_HW_EVENT_MEM_READ			0x06
#define QUADD_ARMV8_HW_EVENT_MEM_WRITE			0x07
#define QUADD_ARMV8_HW_EVENT_EXC_TAKEN			0x09
#define QUADD_ARMV8_HW_EVENT_EXC_EXECUTED		0x0A
#define QUADD_ARMV8_HW_EVENT_CID_WRITE			0x0B
#define QUADD_ARMV8_HW_EVENT_PC_WRITE			0x0C
#define QUADD_ARMV8_HW_EVENT_PC_IMM_BRANCH		0x0D
#define QUADD_ARMV8_HW_EVENT_PC_PROC_RETURN		0x0E
#define QUADD_ARMV8_HW_EVENT_MEM_UNALIGNED_ACCESS	0x0F
#define QUADD_ARMV8_HW_EVENT_TTBR_WRITE			0x1C

/* Common microarchitectural events. */
#define QUADD_ARMV8_HW_EVENT_L1_ICACHE_REFILL		0x01
#define QUADD_ARMV8_HW_EVENT_ITLB_REFILL		0x02
#define QUADD_ARMV8_HW_EVENT_DTLB_REFILL		0x05
#define QUADD_ARMV8_HW_EVENT_MEM_ACCESS			0x13
#define QUADD_ARMV8_HW_EVENT_L1_ICACHE_ACCESS		0x14
#define QUADD_ARMV8_HW_EVENT_L1_DCACHE_WB		0x15
#define QUADD_ARMV8_HW_EVENT_L2_CACHE_ACCESS		0x16
#define QUADD_ARMV8_HW_EVENT_L2_CACHE_REFILL		0x17
#define QUADD_ARMV8_HW_EVENT_L2_CACHE_WB		0x18
#define QUADD_ARMV8_HW_EVENT_BUS_ACCESS			0x19
#define QUADD_ARMV8_HW_EVENT_MEM_ERROR			0x1A
#define QUADD_ARMV8_HW_EVENT_BUS_CYCLES			0x1D

/* ARMv8 Cortex-A57 specific event types. */
#define QUADD_ARMV8_A57_HW_EVENT_L1D_CACHE_REFILL_LD	0x42
#define QUADD_ARMV8_A57_HW_EVENT_L1D_CACHE_REFILL_ST	0x43
#define QUADD_ARMV8_A57_HW_EVENT_L2D_CACHE_REFILL_LD	0x52
#define QUADD_ARMV8_A57_HW_EVENT_L2D_CACHE_REFILL_ST	0x53

#define PMU_INDEX 5

#define PMUREG(name, num) name ## num ## _EL0
#define PMEVCNTR(num) PMUREG(PMEVCNTR, num)
#define PMEVTYPER(num) PMUREG(PMEVTYPER, num)

#define HYPERVISOR_COUNTER_TIMER_PHYSICAL_IRQ_ID	26

#define TIMER_TARGET_START	0
#define TIMER_TARGET_END	1

#define TDMA_STATE_COMPUTING_PHASE			0
#define TDMA_STATE_MEMORY_PHASE				1
#define TDMA_STATE_WAITING_FOR_MEMORY			2
#define TDMA_STATE_BLOCKED_BY_OVERUSAGE			3
#define TDMA_STATE_BLOCKED_BY_OVERUSAGE_ALLOW_NESTED	4


extern void *gicc_base;
extern void *gicd_base;
static u64 tdma_slot_period = UINT64_MAX;


static inline int gicv2_get_prio(int irqn)
{
	u32 prio = mmio_read32(gicd_base + GICD_IPRIORITYR + IRQ_BYTE_OFFSET(irqn));
	return (prio >> IRQ_BYTE_POSITION(irqn)) & IRQ_BYTE_MASK;
}

static inline void gicv2_set_prio(int irqn, int prio)
{
	u32 p = mmio_read32(gicd_base + GICD_IPRIORITYR + IRQ_BYTE_OFFSET(irqn));
	p &= ~(IRQ_BYTE_MASK << IRQ_BYTE_POSITION(irqn));
	p |= (prio & IRQ_BYTE_MASK) << IRQ_BYTE_POSITION(irqn);
	mmio_write32(gicd_base + GICD_IPRIORITYR + IRQ_BYTE_OFFSET(irqn), p);
}

static inline int gicv2_get_targets(int irqn)
{
	u32 t = mmio_read32(gicd_base + GICD_ITARGETSR + IRQ_BYTE_OFFSET(irqn));
	return (t >> IRQ_BYTE_POSITION(irqn)) & IRQ_BYTE_MASK;
}

static inline void gicv2_set_targets(int irqn, int targets)
{
	u32 t = mmio_read32(gicd_base + GICD_ITARGETSR + IRQ_BYTE_OFFSET(irqn));
	t &= ~(IRQ_BYTE_MASK << IRQ_BYTE_POSITION(irqn));
	t |= (targets & IRQ_BYTE_MASK) << IRQ_BYTE_POSITION(irqn);
	mmio_write32(gicd_base + GICD_ITARGETSR + IRQ_BYTE_OFFSET(irqn), t);
}

/* Globally lower (numerically increase) all current priorities and
 * set maximal priority to timer and PMU IRQs */
static inline void tdma_init_priorities(void)
{
	int i;

	for (i = 0; i < CCPLEX_IRQ_SIZE; i++) {
		u32 prio = gicv2_get_prio(i);

		/* Avoid chaning the priorities, which are low enough and never
		 * set minimal (i.e. always masked) priority. */
		while (prio < IRQ_PRIORITY_THR &&
		       prio < IRQ_PRIORITY_MIN - IRQ_PRIORITY_INC)
			prio += IRQ_PRIORITY_INC;
		gicv2_set_prio(i, prio);
	}

	// set priorities for PMU modules
//	for (i = 0; i < ARRAY_SIZE(tx2_cpu_id2irqn); i++) {
//		gicv2_set_prio(tx2_cpu_id2irqn[i], IRQ_PRIORITY_MAX);
//	}

	// set priority for hypervisor timer
	gicv2_set_prio(HYPERVISOR_COUNTER_TIMER_PHYSICAL_IRQ_ID, IRQ_PRIORITY_MAX);
}
//medium
static inline void tdma_print_priorities(void)
{
	int i, j;
	u32 prio;
	for (i = 0; i < CCPLEX_IRQ_SIZE / 4; i++) {
		prio = mmio_read32(gicd_base + GICD_IPRIORITYR + (4 * i));
		for (j = 0; j < 4; j++) {
			printk("%3d %02x\n", i * 4 + j, (prio >> (8 * j)) & 0xFF);
		}
	}
	prio = mmio_read32(gicc_base + GICC_PMR);
	printk("mask: 0x%08x\n", prio);
}

//static void tdma_print_debug( struct per_cpu *cpu_data )
//{
//	printk("TDMA-DEBUG: for core %u: (sid=%d, sta=%u, susf=%u, tt=%u)\n",
//		cpu_data->cpu_id,
//		cpu_data->tdma_slot_id,
//		cpu_data->tdma_state,
//		cpu_data->tdma_suspended_from_state,
//		cpu_data->tdma_timer_target);
//}

//static inline u32 tdma_pmu_count(void)
//{
//	u32 reg32;
//	arm_read_sysreg(PMEVCNTR(PMU_INDEX), reg32);
//	return reg32;
//}
//
//static inline void tdma_pmu_irq_enable(unsigned int cpu_id, u8 targets)
//{
//	int irqn = tx2_cpu_id2irqn[cpu_id];
//
//	/* Enable interrupt for counter at index */
//	arm_write_sysreg(PMINTENSET_EL1, 1 << PMU_INDEX);
//
//	/* Enable PMU interrupt for current core */
//	mmio_write32(gicd_base + GICD_ISENABLER + IRQ_BIT_OFFSET(irqn),
//		     1 << IRQ_BIT_POSITION(irqn));
//
//	gicv2_set_targets(irqn, targets);
//}
//
//static inline void tdma_pmu_irq_disable(unsigned int cpu_id)
//{
//	int irqn = tx2_cpu_id2irqn[cpu_id];
//
//	/* Disable interrupt for counter at index */
//	arm_write_sysreg(PMINTENCLR_EL1, 1 << PMU_INDEX);
//
//	/* Disable PMU interrupt for current core */
//	mmio_write32(gicd_base + GICD_ICENABLER + IRQ_BIT_OFFSET(irqn),
//		     1 << IRQ_BIT_POSITION(irqn));
//}
//
//static inline void tdma_pmu_event_counter_enable(void)
//{
//	arm_write_sysreg(PMCNTENSET_EL0, 1 << PMU_INDEX);
//}
//
//static inline void tdma_pmu_event_counter_disable(void)
//{
//	arm_write_sysreg(PMCNTENCLR_EL0, 1 << PMU_INDEX);
//}
//
//static inline void tdma_pmu_set_budget(u32 budget)
//{
//	arm_write_sysreg(PMEVCNTR(PMU_INDEX), (u32)UINT32_MAX - budget);
//	arm_write_sysreg(PMEVTYPER(PMU_INDEX),
//			 //QUADD_ARMV8_HW_EVENT_BUS_ACCESS
//			 //QUADD_ARMV8_HW_EVENT_CLOCK_CYCLES
//			 QUADD_ARMV8_HW_EVENT_L2_CACHE_REFILL
//		);
//}
//TODO set timer for start of slot
//static void tdma_pmu_isr(struct per_cpu *cpu_data)
//{
//	/* Clear overflow flag */
//	arm_write_sysreg(PMOVSCLR_EL0, 1 << PMU_INDEX);
//
//	/* Block after EOI signalling (done in irqchip.c)*/
//	if(cpu_data->tdma_state == TDMA_STATE_COMPUTING_PHASE || cpu_data->tdma_state == TDMA_STATE_WAITING_FOR_MEMORY) {
//		cpu_data->tdma_suspended_from_state = cpu_data->tdma_state;
//		cpu_data->tdma_state = TDMA_STATE_BLOCKED_BY_OVERUSAGE;
//	} else {
//		printk("TDMA: Core %u had memory overusage in state %u!\n",
//			cpu_data->cpu_id,
//			cpu_data->tdma_state);
//	}
//}

void tdma_block_if_needed(struct per_cpu *cpu_data)
{
//	if (cpu_data->tdma_state == TDMA_STATE_BLOCKED_BY_OVERUSAGE) {
//		u64 spsr, elr;
//
//		/* Do not block while handling other nested IRQs */
//		cpu_data->tdma_state = TDMA_STATE_BLOCKED_BY_OVERUSAGE_ALLOW_NESTED;
//
//		arm_read_sysreg(ELR_EL2, elr);
//		arm_read_sysreg(SPSR_EL2, spsr);
//		asm volatile("msr daifclr, #3"); /* enable IRQs and FIQs */
//
//		printk("TDMA: tdma_block_if_needed: suspending core %u\n", cpu_data->cpu_id);
//
//		/*
//		 * This loop should be race-free. When the timer IRQ
//		 * arrives between the while condition and wfe, it
//		 * sets the Event Register and wfe will not wait.
//		 */
//		while (cpu_data->tdma_state == TDMA_STATE_BLOCKED_BY_OVERUSAGE_ALLOW_NESTED)
//			asm volatile("wfe");
//
//		asm volatile("msr daifset, #3"); /* disable IRQs and FIQs */
//		arm_write_sysreg(ELR_EL2, elr);
//		arm_write_sysreg(SPSR_EL2, spsr);
//	}
}

//static inline void tdma_pmu_init(unsigned int cpu_id)
//{
//	u32 reg32;
//	u64 reg;
//
//	arm_read_sysreg(PMCR_EL0, reg32);
//
//	if (PMU_INDEX != ((reg32 & PMCR_EL0_N_MASK) >> PMCR_EL0_N_POS) - 1) {
//		// This code could be more dynamic. For the moment it only supports
//		// platforms which have exactly #(PMU_INDEX+1) event counters.
//		panic_printk("TDMA: PMU index mismatch\n");
//		panic_stop();
//	}
//
//	/* Reserve a performance counter at index for hypervisor
//	 * (decrease number of accessible counters from EL1 and EL0) */
//	arm_read_sysreg(MDCR_EL2, reg);
//	reg &= ~MDCR_EL2_HPMN_MASK;
//	reg |= MDCR_EL2_HPME + (PMU_INDEX - 1); //BUG? I think the -1 is not necessary (see MDCR_EL2 docs)
//	arm_write_sysreg(MDCR_EL2, reg);
//
//	/* Enable the counter for hypervisor */
//	tdma_pmu_event_counter_disable();
//	arm_write_sysreg(PMOVSCLR_EL0, 1 << PMU_INDEX); // Clear overflow flag
//}

static inline u64 tdma_timer_count(void)
{
	u64 reg64;
	arm_read_sysreg(CNTPCT_EL0, reg64);
	return reg64;
}

static inline void tdma_timer_irq_enable(void)
{
	/* Configure compare value first! (timer >= compare -> isr) */
	u32 reg;
	arm_read_sysreg(CNTHP_CTL_EL2, reg);
	reg &= ~CNTHP_CTL_EL2_IMASK;
	arm_write_sysreg(CNTHP_CTL_EL2, reg);

	mmio_write32(gicd_base + GICD_ISENABLER, (1 << HYPERVISOR_COUNTER_TIMER_PHYSICAL_IRQ_ID));
}

static inline void tdma_timer_irq_disable(void)
{
	u32 reg;
	arm_read_sysreg(CNTHP_CTL_EL2, reg);
	reg |= CNTHP_CTL_EL2_IMASK;
	arm_write_sysreg(CNTHP_CTL_EL2, reg);

	mmio_write32(gicd_base + GICD_ICENABLER, (1 << HYPERVISOR_COUNTER_TIMER_PHYSICAL_IRQ_ID));
}

static inline void tdma_timer_enable(void)
{
	u32 reg;
	arm_read_sysreg(CNTHP_CTL_EL2, reg);
	reg |= CNTHP_CTL_EL2_ENABLE;
	arm_write_sysreg(CNTHP_CTL_EL2, reg);
}

static inline void tdma_timer_set_cmpval(u64 cmp)
{
	arm_write_sysreg(CNTHP_CVAL_EL2, cmp);
}

static inline void tdma_timer_disable(void)
{
	u32 reg;
	arm_read_sysreg(CNTHP_CTL_EL2, reg);
	reg &= ~CNTHP_CTL_EL2_ENABLE;
	arm_write_sysreg(CNTHP_CTL_EL2, reg);
}

static inline void tdma_timer_init(void)
{
	/* Set compare value to 200 years from 0 */
	tdma_timer_set_cmpval(UINT64_MAX);
	tdma_timer_irq_enable();
}
static inline bool tdma_timer_start(struct per_cpu *cpu_data)
{
	const u8 slot_id = cpu_data->tdma_slot_id;
	const u64 cycle_period = tdma_slot_period * tdma_num_slots;
	const u64 timer_count = tdma_timer_count();
	const u8 current_slot_id = (u8)((timer_count / tdma_slot_period) % tdma_num_slots);

	u64 cycle_start_timestamp = (timer_count / cycle_period) * cycle_period;

	bool currently_in_mem_phase = false;
	if(current_slot_id == slot_id) {
		// set the timer to the end of the active slot / the beggining
		// of the next slot
		tdma_timer_set_cmpval(cycle_start_timestamp + (slot_id + 1) * tdma_slot_period);
		cpu_data->tdma_timer_target = TIMER_TARGET_END;
		currently_in_mem_phase = true;
	} else if(current_slot_id < slot_id) {
		// set the timer to the start of the upcomming slot
		tdma_timer_set_cmpval(cycle_start_timestamp + slot_id * tdma_slot_period);
		cpu_data->tdma_timer_target = TIMER_TARGET_START;
	} else {
		// set the timer to the start of the following slot
		tdma_timer_set_cmpval(cycle_start_timestamp + slot_id * tdma_slot_period + cycle_period);
		cpu_data->tdma_timer_target = TIMER_TARGET_START;
	}

	tdma_timer_enable();

	return currently_in_mem_phase;
}

static void tdma_timer_isr(struct per_cpu *cpu_data)
{
	if (cpu_data->tdma_timer_target == TIMER_TARGET_START) {

		if(cpu_data->tdma_state == TDMA_STATE_BLOCKED_BY_OVERUSAGE) {

			if(cpu_data->tdma_suspended_from_state == TDMA_STATE_COMPUTING_PHASE) {

				//printk("TDMA: Resuming suspended core %u in C-Phase.\n", cpu_data->cpu_id);

				// reset memory protection
				//tdma_pmu_event_counter_disable();
				//tdma_pmu_set_budget(PMU_BUDGET);
				//tdma_pmu_event_counter_enable();
			}

			cpu_data->tdma_state = cpu_data->tdma_suspended_from_state;
		}

		if(cpu_data->tdma_state == TDMA_STATE_WAITING_FOR_MEMORY) {

			//printk("TDMA: Signaling start of memory phase to core %u.\n", cpu_data->cpu_id);

			// send SGI to core slot_start
			// disable memory protection
			//tdma_pmu_event_counter_disable();

			// start timer for end of slot interrupt
			tdma_timer_start(cpu_data);
			cpu_data->tdma_state = TDMA_STATE_MEMORY_PHASE;

		}

	} else { // TIMER_TARGET_END

		if(cpu_data->tdma_state != TDMA_STATE_MEMORY_PHASE) {
			printk("TDMA: Core %u got a slot end interrupt even though it is not in M-Phase!\n", cpu_data->cpu_id);
			return;
		}

		//printk("TDMA: Signaling end of memory phase to core %u.\n", cpu_data->cpu_id);

		// send SGI to core slot_end
		irqchip_set_pending(cpu_data, JAILHOUSE_MEMORY_ARBITRATION_SGI_MEMORY_PHASE_ENDED);

		// enable memory protection
		//tdma_pmu_set_budget(PMU_BUDGET);
		//tdma_pmu_event_counter_enable();

		// start timer for start of slot interrupt
		tdma_timer_start(cpu_data);
		cpu_data->tdma_state = TDMA_STATE_WAITING_FOR_MEMORY;
		
	}
}

//static bool is_pmu_irq(struct per_cpu *cpu_data, u32 irqn)
//{
//	u32 reg;
//
//	if (irqn != tx2_cpu_id2irqn[cpu_data->cpu_id])
//		return false;
//
//	arm_read_sysreg(PMOVSCLR_EL0, reg);
//
//	return (reg & (1 << PMU_INDEX)) != 0;
//}

bool tdma_handle_interrupt(struct per_cpu *cpu_data, u32 irqn)
{
//	if (is_pmu_irq(cpu_data, irqn)) {
//		tdma_pmu_isr(cpu_data);
//		return true;
//	} else if
	if (irqn == HYPERVISOR_COUNTER_TIMER_PHYSICAL_IRQ_ID) {
		tdma_timer_isr(cpu_data);
		return true;
	}

	return false;
}


void tdma_setup_irq_target(u8 local_irq_target)
{
	struct per_cpu *cpu_data = this_cpu_data();

	if(this_cell() == &root_cell) {
		printk("Ignoring root cell!\n");
		return;
	}

	(void)local_irq_target; // currently unused
	(void)cpu_data; // currently unused
//	tdma_pmu_init(cpu_data->cpu_id);
//	tdma_pmu_irq_enable(cpu_data->cpu_id, local_irq_target);
	tdma_timer_init();

	/* Interrupt controller can filter interrupts with lower priorities
	 * lower number = higher priority
	 * Described in detail in Interrupt prioritization
	 * ARM GIC Architecture Specification
	 * TODO: Implement priorities */

	/* TODO: Do this only on one core */
	tdma_init_priorities();
}

void tdma_on_cell_create( struct cell *cell )
{
	struct per_cpu *cpu_data = per_cpu(first_cpu(cell->cpu_set));

	cpu_data->tdma_slot_id = -1;
	cpu_data->tdma_state = TDMA_STATE_COMPUTING_PHASE;
	cpu_data->tdma_suspended_from_state = TDMA_STATE_COMPUTING_PHASE;
	cpu_data->tdma_timer_target = 0;

	if(cell == &root_cell) {
		printk("TDMA: Not enabling TDMA on root cell CPU %d.\n", cpu_data->cpu_id);
		return;
	}

	u32 freq;
	arm_read_sysreg(CNTFRQ_EL0, freq);
	tdma_slot_period = (tdma_slot_period_us * freq) / S_TO_US(1);

	printk("TDMA: Initializing CPU %u of cell \"%s\".\n",
		cpu_data->cpu_id,
		cell->config->name);

	// slot_id is assigned by cell id
	cpu_data->tdma_slot_id = cell->config->id-1;

	if(cpu_data->tdma_slot_id >= tdma_num_slots) {
		printk("TDMA: Initializing CPU %u of cell \"%s\" failed: cell id larger than number of slots!\n",
			cpu_data->cpu_id,
			cell->config->name);
		cpu_data->tdma_slot_id = -1;
		return;
	}
	printk("TDMA: Slot %d assigned to CPU %u of cell \"%s\".\n",
		cpu_data->tdma_slot_id,
		cpu_data->cpu_id,
		cell->config->name);
}

//medium: This will not free the used slot_id
void tdma_on_cell_destroy( struct cell *cell )
{
	struct per_cpu *cpu_data = per_cpu(first_cpu(cell->cpu_set));

	if(cell == &root_cell) {
		printk("TDMA: Not deinitializing CPU %u of root cell.\n", cpu_data->cpu_id);
		return;
	}

	printk("TDMA: Deinitializing CPU %u of cell \"%s\".\n",
			cpu_data->cpu_id,
			cell->config->name);


//	tdma_pmu_event_counter_disable();
	tdma_timer_disable();

//	tdma_pmu_irq_disable(cpu_data->cpu_id);
	tdma_timer_irq_disable();

	/* Free the event counter */
	u32 reg32;
	arm_read_sysreg(MDCR_EL2, reg32);
	reg32 &= ~(MDCR_EL2_HPMN_MASK);
	reg32 |= MDCR_EL2_HPME + PMU_INDEX + 1;
	arm_write_sysreg(MDCR_EL2, reg32);
}

/**
 * Hypercall called on memory phase borders
 */
long tdma_hypercall(struct per_cpu *cpu_data, unsigned long action_code)
{
	//printk("TDMA: hypercall on core %u, action code %lu .. ", cpu_data->cpu_id, action_code);

	switch(action_code) {

		case JAILHOUSE_MEMORY_ARBITRATION_BEGIN_MEM_PHASE:

			if(cpu_data->tdma_state == TDMA_STATE_COMPUTING_PHASE) {

				// start timer for next slot start or end
				bool currently_in_mem_phase = tdma_timer_start(cpu_data);

				if(currently_in_mem_phase == true) {

					// disable memory protection
					//tdma_pmu_event_counter_disable();

					cpu_data->tdma_state = TDMA_STATE_MEMORY_PHASE;

					return JAILHOUSE_MEMORY_ARBITRATION_ACK;
				} else {

					cpu_data->tdma_state = TDMA_STATE_WAITING_FOR_MEMORY;

					return JAILHOUSE_MEMORY_ARBITRATION_NACK;
				}
			} else {

				printk("TDMA: Core %u requested beginning of memory phase while in TDMA state %u! (Ignored)\n",
					cpu_data->cpu_id,
					cpu_data->tdma_state);

				return JAILHOUSE_MEMORY_ARBITRATION_INVALID;
			}
			break;
		case JAILHOUSE_MEMORY_ARBITRATION_END_MEM_PHASE:

			if(cpu_data->tdma_state == TDMA_STATE_COMPUTING_PHASE) {

				printk("TDMA: Core %u requested end of memory phase while in TDMA state %u! (Ignored)\n",
					cpu_data->cpu_id,
					cpu_data->tdma_state);

				return JAILHOUSE_MEMORY_ARBITRATION_INVALID;
			} else { // TDMA_STATE_MEMORY_PHASE || TDMA_STATE_WAITING_FOR_MEMORY

				// disable timer
				tdma_timer_disable();

				// enable memory protection
				//tdma_pmu_set_budget(PMU_BUDGET);
				//tdma_pmu_event_counter_enable();

				cpu_data->tdma_state = TDMA_STATE_COMPUTING_PHASE;

				return JAILHOUSE_MEMORY_ARBITRATION_ACK;
			}
			break;
		default:

			printk("TDMA: Core %u send invalid TDMA action code %lu!\n", cpu_data->cpu_id, action_code);

			return JAILHOUSE_MEMORY_ARBITRATION_INVALID;
	}

	return JAILHOUSE_MEMORY_ARBITRATION_INVALID;
}

#endif /* CONFIG_JAILHOUSE_TDMA */
