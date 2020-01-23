/*
 * PREM and Memguard functionalities
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */
#ifndef HERCULES_GPREM_H
#define HERCULES_GPREM_H

/* These codes MUST be equal to jailhouse/hypercall.h */
#define JAILHOUSE_HC_PREM_CODE 9
#define JAILHOUSE_HC_MEMGUARD_CODE 10

enum prem_phase {
        PREM_COMPATIBLE = 0,
        PREM_MEMORY     = 1,
        PREM_COMPUTE    = 2,
};

/* Memguard flags */
/* Chooses between periodic or one-shot budget replenishment */
#define MGF_PERIODIC      (1 << 0)
/* Mask (disable) low priority interrupts until next memguard call */
#define MGF_MASK_INT      (1 << 1)

typedef struct
{
    unsigned long budget_time;
    unsigned long budget_memory;
    unsigned long flags;
}mem_configuration;

/* Interfaces */
__u64 gprem_set(enum prem_phase phase, unsigned long mem_budget, unsigned long timeout,
    unsigned long period, unsigned long flags);
__u64 memguard_call(unsigned long budget_time, unsigned long budget_memory,
    unsigned long flags);
char* PHASE(enum prem_phase phase);


__u64 gprem_set(enum prem_phase phase, unsigned long mem_budget, unsigned long timeout,
            unsigned long period, unsigned long flags)
{
        return jailhouse_call_arg5(JAILHOUSE_HC_PREM_CODE, phase, mem_budget, timeout, period, flags);
}

__u64 memguard_call(unsigned long budget_time, unsigned long budget_memory,
 		   unsigned long flags)
{
        return jailhouse_call_arg3(JAILHOUSE_HC_MEMGUARD, budget_time, budget_memory, flags);
}

/* Return phase string */
char* PHASE(enum prem_phase phase)
{
        switch (phase) {
                case PREM_COMPATIBLE:
                        return "PREM_COMPATIBLE";
                        break;
                case PREM_MEMORY:
                        return "PREM_MEMORY";
                        break;
                case PREM_COMPUTE:
                        return "PREM_COMPUTE";
                        break;
                default:
                        return "ERROR";
                        break;
        }

        return "ERR";
}

#endif /* HERCULES_GPREM_H */
