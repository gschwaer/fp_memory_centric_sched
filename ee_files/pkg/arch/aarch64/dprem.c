#include "dprem.h"

#include "ee_internal.h"
#include "ee_oo_api_osek.h"

#include "jailhouse/hypercall.h"
#include "asm/memory_arbitration.h"

#define MAX_TASK_PRIORITY (OSEE_ISR2_PRIO_BIT-1)

TaskType memory_phase_started_isr_id;
TaskType memory_phase_ended_isr_id;


FUNC(void, OS_CODE)
DPREM_suspend_running_task
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
);

FUNC(OsEE_bool, OS_CODE)
DPREM_suspend_highest_priority_active_task_from_ISR
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
);

FUNC(OsEE_bool, OS_CODE)
DPREM_resume_highest_priority_task_from_ISR
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
);


// for some reason the compiler wants to see this as int instead of long
// if you use long e.g. for __r0 the hypervisor will not receive the correct value
static int hvc(unsigned int r0, unsigned int r1, unsigned int r2)
{
	register unsigned int __r0 asm("r0") = r0;
	register unsigned int __r1 asm("r1") = r1;
	register unsigned int __r2 asm("r2") = r2;

	asm volatile("hvc #0x4a48" : "=r" (__r0) : "r" (__r0), "r" (__r1), "r" (__r2));
	return __r0;
}

static void printk_sched_node_info(struct OsEE_SN_tag* p_sn)
{
	while(p_sn != NULL) {
		if(p_sn->p_tdb == NULL) {
			printk("tdb==NULL, ");
		} else if(p_sn->p_tdb->p_tcb == NULL) {
			printk("tcb==NULL, ");
		} else {
			printk("%lu(rp=%lu,cp=%lu,s=%lu), ",
					p_sn->p_tdb->tid,
					p_sn->p_tdb->ready_prio,
					p_sn->p_tdb->p_tcb->current_prio,
					p_sn->p_tdb->p_tcb->status);
		}
		p_sn = p_sn->p_next;
	}
	printk("-\n");
}

static void print_scheduler_queues(void)
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb          = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb          = p_cdb->p_ccb;
	CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA) p_curr         = p_ccb->p_curr;

	printk("Erika: I'm %lu:\n", p_curr->tid);

	printk("       * Stacked Queue: ");
	printk_sched_node_info(p_ccb->p_stk_sn);

	printk("       * Run Queue: ");
	printk_sched_node_info(p_ccb->rq);

	printk("       * Suspended Queue: ");
	printk_sched_node_info(p_ccb->p_suspended_sn);
}

static void list_remove(struct OsEE_SN_tag** p_list_sn, struct OsEE_SN_tag* p_sn)
{
	if(*p_list_sn == p_sn) {
		*p_list_sn = p_sn->p_next;
	} else {
		struct OsEE_SN_tag* p_curr_sn = *p_list_sn;
		while(p_curr_sn != NULL) {
			if(p_curr_sn->p_next == p_sn) {
				p_curr_sn->p_next = p_sn->p_next;
				return;
			}
			p_curr_sn = p_curr_sn->p_next;
		}
	}
}

static void list_push_to_head(struct OsEE_SN_tag** p_list_sn, struct OsEE_SN_tag* p_sn)
{
	p_sn->p_next = *p_list_sn;
	*p_list_sn = p_sn;
}

// if the list is null, insert as first element.
// if the reference element is not found or NULL, insert at the end of the list.
static void list_insert_before(struct OsEE_SN_tag** p_list_sn, struct OsEE_SN_tag* p_sn, struct OsEE_SN_tag* p_ref_sn)
{
	if(*p_list_sn == NULL) {
		p_sn->p_next = NULL;
		*p_list_sn = p_sn;
		return;
	}

	struct OsEE_SN_tag* p_curr_sn = *p_list_sn;
	while(p_curr_sn != NULL) {
		if(p_curr_sn->p_next == p_ref_sn || p_curr_sn->p_next == NULL) {
			p_sn->p_next = p_curr_sn->p_next;
			p_curr_sn->p_next = p_sn;
			return;
		}
		p_curr_sn = p_curr_sn->p_next;
	}
}
#define list_push_to_tail(list, item) list_insert_before((list), (item), NULL)

FUNC(OsEE_bool, OS_CODE)
DPREM_suspend_highest_priority_active_task_from_ISR
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
)
{
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// find highest priority regular task in stacked queue
	struct OsEE_SN_tag* p_curr_sn = p_ccb->p_stk_sn;
	struct OsEE_SN_tag* p_highest_priority_stacked_sn = NULL;
	while(p_curr_sn != NULL) {
		OsEE_TDB *p_curr_tdb = p_curr_sn->p_tdb;
		OsEE_TCB *p_curr_tcb = p_curr_tdb->p_tcb;
		bool curr_sn_is_regular_task = p_curr_tdb->task_type == OSEE_TASK_TYPE_BASIC ||
									   p_curr_tdb->task_type == OSEE_TASK_TYPE_EXTENDED;
		bool curr_sn_is_highest_priority = p_highest_priority_stacked_sn == NULL ||
										   p_curr_tcb->current_prio > p_highest_priority_stacked_sn->p_tdb->p_tcb->current_prio;
		if(curr_sn_is_regular_task && curr_sn_is_highest_priority) {
			p_highest_priority_stacked_sn = p_curr_sn;
		}
		p_curr_sn = p_curr_sn->p_next;
	}

	// move highest priority task from stacked queue to suspended queue
	if(p_highest_priority_stacked_sn != NULL) {
		list_remove(&p_ccb->p_stk_sn, p_highest_priority_stacked_sn);
		list_push_to_head(&p_ccb->p_suspended_sn, p_highest_priority_stacked_sn);
		return OSEE_M_TRUE;
	}

	return OSEE_M_FALSE;
}

FUNC(OsEE_bool, OS_CODE)
DPREM_resume_highest_priority_task_from_ISR
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
)
{
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// find highest priority task in suspended queue
	struct OsEE_SN_tag* p_curr_sn = p_ccb->p_suspended_sn;
	struct OsEE_SN_tag* p_highest_priority_suspended_sn = NULL;
	while(p_curr_sn != NULL) {
		OsEE_TCB *p_curr_tcb = p_curr_sn->p_tdb->p_tcb;
		bool curr_sn_is_highest_priority = p_highest_priority_suspended_sn == NULL ||
										   p_curr_tcb->current_prio > p_highest_priority_suspended_sn->p_tdb->p_tcb->current_prio;
		if(curr_sn_is_highest_priority) {
			p_highest_priority_suspended_sn = p_curr_sn;
		}
		p_curr_sn = p_curr_sn->p_next;
	}

	// find first regular task in stacked queue
	p_curr_sn = p_ccb->p_stk_sn;
	struct OsEE_SN_tag* p_first_regular_stacked_sn = NULL;
	while(p_curr_sn != NULL) {
		bool curr_sn_is_regular_task = p_curr_sn->p_tdb->task_type == OSEE_TASK_TYPE_BASIC ||
									   p_curr_sn->p_tdb->task_type == OSEE_TASK_TYPE_EXTENDED;
		if(curr_sn_is_regular_task) {
			p_first_regular_stacked_sn = p_curr_sn;
			break;
		}
		p_curr_sn = p_curr_sn->p_next;
	}

	// move highest priority task from suspended queue to the head of regular tasks in the stacked queue
	if(p_highest_priority_suspended_sn != NULL) {
		// set priority to ceiling priority (might be necessary if the initial memory access request was denied)
		p_highest_priority_suspended_sn->p_tdb->p_tcb->current_prio = MAX_TASK_PRIORITY;

		list_remove(&p_ccb->p_suspended_sn, p_highest_priority_suspended_sn);
		if(p_first_regular_stacked_sn == NULL) {
			list_push_to_tail(&p_ccb->p_stk_sn, p_highest_priority_suspended_sn);
		} else {
			list_insert_before(&p_ccb->p_stk_sn, p_highest_priority_suspended_sn, p_first_regular_stacked_sn);
		}
		return OSEE_M_TRUE;
	}

	return OSEE_M_FALSE;
}

FUNC(void, OS_CODE)
DPREM_suspend_running_task
(
		P2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA)  p_cdb
)
{
	// when called we should be in an atomic section & core structs should be locked
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb      = p_cdb->p_ccb;
	CONSTP2VAR(OsEE_SN, AUTOMATIC, OS_APPL_DATA)  p_stk_sn   = p_ccb->p_stk_sn;
	CONSTP2VAR(OsEE_TDB, AUTOMATIC, OS_APPL_DATA) p_curr_tdb = p_ccb->p_curr;
	CONSTP2VAR(OsEE_TCB, AUTOMATIC, OS_APPL_DATA) p_curr_tcb = p_curr_tdb->p_tcb;

	if(p_stk_sn != NULL) {
		if( p_stk_sn->p_tdb->task_type == OSEE_TASK_TYPE_BASIC ||
			p_stk_sn->p_tdb->task_type == OSEE_TASK_TYPE_EXTENDED ) {

			// pop task from stacked queue and add to suspended queue
			p_curr_tcb->p_own_sn = osEE_scheduler_core_pop_running(p_cdb, &p_ccb->rq);
			list_push_to_head(&p_ccb->p_suspended_sn, p_curr_tcb->p_own_sn);

			// we need to set the task to READY_STACKED for it to be restored correctly later
			p_curr_tcb->status = OSEE_TASK_READY_STACKED;
			p_curr_tcb->current_prio = p_curr_tdb->ready_prio;

			osEE_unlock_core(p_cdb);
			osEE_change_context_from_running(p_curr_tdb, p_ccb->p_curr);
			osEE_lock_core(p_cdb);
		} else {
			printk("Erika: DPREM_suspend_running_task called from isr? (ignored)\n");
			print_scheduler_queues();
		}
	} else {
		printk("Erika: DPREM_suspend_running_task called from idle? (ignored)\n");
		print_scheduler_queues();
	}
}

static void memory_phase_started_ISR( void )
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// begin atomic & lock core structs
	CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();
	osEE_lock_core(p_cdb);

	if(p_ccb->mem_arbit_status == MEMORY_TOKEN_REQUESTED) {
		p_ccb->mem_arbit_status = MEMORY_TOKEN_ACQUIRED;
		OsEE_bool ret = DPREM_resume_highest_priority_task_from_ISR(p_cdb);
		if(ret == OSEE_M_FALSE) {
			printk("Erika: memory_phase_started_ISR should resume task, but there was none!\n");
		}
	}

	// end atomic & unlock core structs
	osEE_unlock_core(p_cdb);
	osEE_end_primitive(flags);
}

static void memory_phase_ended_ISR( void )
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// begin atomic & lock core structs
	CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();
	osEE_lock_core(p_cdb);

	//printk("Erika: memory_phase_ended_ISR\n");

	if(p_ccb->mem_arbit_status == MEMORY_TOKEN_ACQUIRED) {
		p_ccb->mem_arbit_status = MEMORY_TOKEN_REQUESTED;
		OsEE_reg ret = DPREM_suspend_highest_priority_active_task_from_ISR(p_cdb);
		if(ret == OSEE_M_FALSE) {
			printk("Erika: memory_phase_ended_ISR should suspend task, but there was none!\n");
		}
	}

	// end atomic & unlock core structs
	osEE_unlock_core(p_cdb);
	osEE_end_primitive(flags);
}


// === API functions ===

FUNC(void, OS_CODE)
DPREM_init
(
		void
)
{
	StatusType ret = E_OK;

	ret |= CreateTask( &memory_phase_started_isr_id, OSEE_TASK_TYPE_ISR2, memory_phase_started_ISR, 1U, 1U, 1U, OSEE_SYSTEM_STACK );
	ret |= CreateTask( &memory_phase_ended_isr_id, OSEE_TASK_TYPE_ISR2, memory_phase_ended_ISR, 1U, 1U, 1U, OSEE_SYSTEM_STACK );

	ret |= SetISR2Source( memory_phase_started_isr_id, JAILHOUSE_MEMORY_ARBITRATION_SGI_MEMORY_PHASE_STARTED );
	ret |= SetISR2Source( memory_phase_ended_isr_id, JAILHOUSE_MEMORY_ARBITRATION_SGI_MEMORY_PHASE_ENDED );

	if(ret != E_OK) {
		printk("Erika: DPREM Initialization failed!");
		return;
	}
}

FUNC(void, OS_CODE)
DPREM_begin_memory_phase(
		void
)
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// begin atomic & lock core structs
	CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();
	osEE_lock_core(p_cdb);

	while(p_ccb->mem_arbit_status != MEMORY_TOKEN_ACQUIRED) {
		if(p_ccb->mem_arbit_status != MEMORY_TOKEN_REQUESTED) {
			int64_t hvc_res = hvc(JAILHOUSE_HC_MEMORY_ARBITRATION, JAILHOUSE_MEMORY_ARBITRATION_BEGIN_MEM_PHASE, 0);
			if(hvc_res == JAILHOUSE_MEMORY_ARBITRATION_ACK) {
				p_ccb->mem_arbit_status = MEMORY_TOKEN_ACQUIRED;
			} else if(hvc_res == JAILHOUSE_MEMORY_ARBITRATION_NACK) {
				p_ccb->mem_arbit_status = MEMORY_TOKEN_REQUESTED;
			} else {
				printk("Erika: hvc(TDMA_HYPERCALL_ACTION_BEGIN_MEM_PHASE) responded with %ld\n", hvc_res);
			}
			continue;
		}
		// requested but not acquired -> suspend current task
		DPREM_suspend_running_task(p_cdb);
	}

	// set priority to ceiling priority
	p_ccb->p_curr->p_tcb->current_prio = MAX_TASK_PRIORITY;

	// end atomic & unlock core structs
	osEE_unlock_core(p_cdb);
	osEE_end_primitive(flags);
}

FUNC(OsEE_reg, OS_CODE)
DPREM_end_memory_phase(
		void
)
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// begin atomic & lock core structs
	CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();
	osEE_lock_core(p_cdb);

	int64_t hvc_res = hvc(JAILHOUSE_HC_MEMORY_ARBITRATION, JAILHOUSE_MEMORY_ARBITRATION_END_MEM_PHASE, 0);
	if(hvc_res == JAILHOUSE_MEMORY_ARBITRATION_ACK) {
		p_ccb->mem_arbit_status = MEMORY_TOKEN_NONE;
	} else {
		printk("Erika: hvc(TDMA_HYPERCALL_ACTION_END_MEM_PHASE) responded with %ld\n", hvc_res);
	}


	// end atomic & unlock core structs
	osEE_unlock_core(p_cdb);
	// don't enable interrupts for the execution phase to be not interrupted
	//osEE_end_primitive(flags);
	return flags;
}

FUNC(void, OS_CODE)
DPREM_end_execution_phase(
		OsEE_reg flags
)
{
	CONSTP2VAR(OsEE_CDB, AUTOMATIC, OS_APPL_DATA) p_cdb = osEE_get_curr_core();
	CONSTP2VAR(OsEE_CCB, AUTOMATIC, OS_APPL_DATA) p_ccb = p_cdb->p_ccb;

	// begin atomic & lock core structs
	//CONST(OsEE_reg, AUTOMATIC) flags = osEE_begin_primitive();
	osEE_lock_core(p_cdb);

	// restore priority to dispatch priority
	p_ccb->p_curr->p_tcb->current_prio = p_ccb->p_curr->dispatch_prio;

	// end atomic & unlock core structs
	osEE_unlock_core(p_cdb);
	osEE_end_primitive(flags);
}
