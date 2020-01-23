/*
 * HERCULES simple test
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */

#include <inmate.h>
#include "sleep.h"
#include "gprem.h"
#define HERCULES_DEBUG
#include "utilities.h"

#define LOOP_NUM 1000
#define ARRAY_SIZE ((4*1024*1024)/sizeof(int))

/* PREM call tunable parameters */
unsigned long budget = 100000000;
unsigned long timeout = 1000000;
unsigned long flags = MGF_PERIODIC;


void inmate_main(void)
{
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    u64 delta,i = 0;

    DEBUG_PRINT(("\n\n### HERCULES TEST INMATE 0 ###\n"));
    DEBUG_PRINT(("Sleeping for 6 seconds\n"));
    sleep(6);

    DEBUG_PRINT(("Taking the lock....\n"));
    // Try to take the lock
    delta = timer_get_ticks();
    gprem_set(PREM_MEMORY,1000,1000,1000,1);
    delta = timer_get_ticks() - delta;
    DEBUG_PRINT(("I have acquired the lock in %6ld ns\n",\
                            (long)timer_ticks_to_ns(delta)));
    DEBUG_PRINT(("Releasing the lock\n"));
    gprem_set(PREM_COMPUTE, 0, 0, 0, 0);

    DEBUG_PRINT(("Starting PREM_COMPATIBLE - PREM_COMPUTE loops (%d)\n", LOOP_NUM));

    while(i < LOOP_NUM)
    {
        gprem_set(PREM_COMPATIBLE,budget, timeout, timeout, flags);
        delta = timer_get_ticks();
        memcpy(&dst,&src, ARRAY_SIZE);
        delta = timer_get_ticks() - delta;
        gprem_set(PREM_COMPUTE, 0, 0, 0, 0);
        DEBUG_PRINT(("%6ld\n", (long)timer_ticks_to_ns(delta)));

        //sleep(1);
        i++;
    }

    DEBUG_PRINT(("##### TEST ENDED - halt() #####\n"));
    halt();
}
