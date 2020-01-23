#include "hardware_setup_inc.h"

.global get_cycle_counter_value
.type get_cycle_counter_value, %function
.section .text
.func
get_cycle_counter_value:
	mrs   x0, PMCCNTR_EL0;
	ret
.endfunc


.global disable_prefetcher
.type disable_prefetcher, %function
.section .text
.func
disable_prefetcher:
	/* CPU Extended Control Register */
	mrs   x9, CPUECTLR_EL1
    bic   x9, x9, #L2_LDR_STR_DATA_PREF_DIST_MASK // disable load/store data prefetch
    bic   x9, x9, #L2_INSTR_FETCH_PREF_DIST_MASK // disable instruction prefetch
    orr   x9, x9, #TABLE_WALK_DESCR_ACC_PREF // disable table walk descriptor access prefetch
    msr   CPUECTLR_EL1, x9

	/* flush pipeline */
	dsb   SY
	isb

	ret
.endfunc

.global disable_predictor
.type disable_predictor, %function
.section .text
.func
disable_predictor:
	/* CPU Extended Control Register */
	mrs   x9, CPUECTLR_EL1
    bic   x9, x9, #L2_LDR_STR_DATA_PREF_DIST_MASK // disable load/store data prefetch
    bic   x9, x9, #L2_INSTR_FETCH_PREF_DIST_MASK // disable instruction prefetch
    orr   x9, x9, #TABLE_WALK_DESCR_ACC_PREF // disable table walk descriptor access prefetch
    msr   CPUECTLR_EL1, x9

	/* flush pipeline */
	dsb   SY
	isb

	ret
.endfunc

.global enable_cycle_counter
.type enable_cycle_counter, %function
.section .text
// PMUSERENR_EL0   https://developer.arm.com/docs/ddi0595/latest/aarch64-system-registers/PMUSERENR_EL0
// PMCNTENSET_EL0  https://developer.arm.com/docs/ddi0595/latest/aarch64-system-registers/PMCNTENSET_EL0
// PMCCFILTR_EL0   https://developer.arm.com/docs/ddi0595/latest/aarch64-system-registers/PMCCFILTR_EL0
// PMCR_EL0        https://developer.arm.com/docs/ddi0595/latest/aarch64-system-registers/PMCR_EL0
.func
enable_cycle_counter:
	/* Performance Monitors User Enable Register */
	mrs   x9, PMUSERENR_EL0
	orr   x9, x9, #PMUSERENR_CR_BIT // enables access to cycle counter at EL0
	orr   x9, x9, #PMUSERENR_EN_BIT // enables access from EL0 to all of the registers specified above
	msr   PMUSERENR_EL0, x9
	/* Performance Monitors Count Enable Set register */
	mrs   x9, PMCNTENSET_EL0
	orr   x9, x9, #PMCNTENSET_C_BIT // enables the cycle counter
	msr   PMCNTENSET_EL0, x9
	/* Performance Monitors Cycle Count Filter Register */
	mrs   x9, PMCCFILTR_EL0
	bic   x9, x9, PMCCFILTR_U_BIT	// count cycles in EL0
	bic   x9, x9, PMCCFILTR_NSU_BIT // count cycles in Non-secure EL0 (==U)
	bic   x9, x9, PMCCFILTR_P_BIT	// count cycles in EL1
	bic   x9, x9, PMCCFILTR_NSK_BIT // count cycles in Non-secure EL1 (==P)
	orr   x9, x9, PMCCFILTR_NSH_BIT // count cycles in EL2
	bic   x9, x9, PMCCFILTR_SH_BIT	// count cycles in Secure EL2 (!=NSH)
	bic   x9, x9, PMCCFILTR_M_BIT	// count cycles in Secure EL3 (==P)
	msr   PMCCFILTR_EL0, x9
	/* Performance Monitors Control Register */
	mrs   x9, PMCR_EL0
	orr   x9, x9, PMCR_LC_BIT		// cycle counter overflows on unsigned 64 bit overflow
	bic   x9, x9, PMCR_DP_BIT		// allow cycle counting when event counting is prohibited
	bic   x9, x9, PMCR_D_BIT		// enables cycle counting on every clock cycle (opposed to every 64th cycle)
	orr   x9, x9, PMCR_C_BIT		// resets cycle counter to zero
	orr   x9, x9, PMCR_E_BIT		// enables all counters
	msr   PMCR_EL0, x9

	/* flush pipeline */
	dsb   SY
	isb

	ret
.endfunc
