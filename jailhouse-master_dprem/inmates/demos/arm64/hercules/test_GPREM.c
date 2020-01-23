/*
* HERCULES PREM test
*
* Copyright (C) 2018 Università di Modena e Reggio Emilia
*
* Authors:
*  Luca Miccio <lucmiccio@gmail.com>
*
* N.B: this file should be used for testing only
*/
#include <inmate.h>
#include "gprem.h"
#include "sleep.h"
#include "caches.h"

#define COLORED_CACHE_SIZE 524288
#define STD_CACHE_SIZE 2097152

#define SAMPLES 25

#define HERCULES_DEBUG
// Cache size in bytes
#define CACHE_SIZE COLORED_CACHE_SIZE
// Multiplier factor for ARRAY_SIZE (K*CACHE_SIZE)
#define K (2)
// Cache line size in bytes
#define CACHE_LINE_SIZE 64

typedef int line[CACHE_LINE_SIZE/sizeof(int)];
#define ARRAY_SIZE ((CACHE_SIZE*K)/sizeof(line))
#define PART_SIZE (ARRAY_SIZE/K)
#define INF (unsigned long long)(0xffffffffffffffff)
extern void arm_dcaches_flush(void* addr, unsigned int size, int flush);

#include "utilities.h"

unsigned long count_sample = 0;

#define TEST_NO_PREM (1<<0)
#define TEST_PREM (1<<1)
#define ALL (TEST_PREM) | (TEST_NO_PREM)
#define CHECK_BIT(var,flag) ((var) & (flag))

/*static void prefetch_array(void*array, int size)
{
    while(size-- > 0)
        prefetch(array++);
}*/

static void flush_array(void*array, int size)
{
    arm_dcaches_flush(array, size, 0);
}

static void inline reset(void*src)
{
    // Reset and clean all
    count_sample = 0;
    flush_i_cache();
    flush_array(src,ARRAY_SIZE);
}

void inmate_main(void)
{
    line src[ARRAY_SIZE];
    int i,k,j;
    u64 delta;
    unsigned test_type = (unsigned)cmdline_parse_int("type", ALL);
    volatile u64 sum = 0;

    sleep(7);
    DEBUG_PRINT(("\n\n#### HERCULES PREM TEST ####\n"));
    DEBUG_PRINT(("CACHE SIZE: %llu \n", CACHE_SIZE));
    DEBUG_PRINT(("ARRAY(item) SIZE (#C*%d): %llu (%llu)\n", K, ARRAY_SIZE, sizeof(line)));
    DEBUG_PRINT(("PART SUB-ARRAY SIZE: %llu\n",PART_SIZE));
    DEBUG_PRINT(("Test type: %d, samples: %d\n",test_type, SAMPLES));

    reset(src);

    if (CHECK_BIT(test_type,TEST_NO_PREM)){
        DEBUG_PRINT(("### NO PREM TEST ###\n"));
        while(count_sample < SAMPLES){
            sum = 0;
            delta = timer_get_ticks();
            for (i = 0; i < ARRAY_SIZE; i++)
                for(k = 0; k < ARRAY_SIZE; k++){
                    sum += src[k][0];
                    //heartbeat();
                }
            delta = timer_get_ticks() - delta;
            DEBUG_PRINT(("Time(ns): "));
            printk("%llu\n",delta*32);
            //printk("%llu\n",sum);
            count_sample++;
    	}
        printk("\n");
    }

    reset(src);

    if (CHECK_BIT(test_type,TEST_PREM)) {
        /*DEBUG_PRINT(("### PREM TEST (pref+flush) ###\n"));
        while(count_sample < SAMPLES){
            sum = 0;
            delta = timer_get_ticks();
            for (j = 0; j < K; j++) {
                gprem_set(PREM_MEMORY,INF,INF,INF,MGF_PERIODIC);
                prefetch_array(&src[j*PART_SIZE],PART_SIZE*sizeof(line));
                for (i = 0; i < ARRAY_SIZE; i++) {
                    for(k = 0; k < PART_SIZE; k++) {
                        int index = (j*PART_SIZE) + k;
                        sum += src[index][0];
                        //heartbeat();
                    }
                }
                flush_array(&src[j*PART_SIZE],PART_SIZE*sizeof(line));
                gprem_set(PREM_COMPUTE,1677,INF,10,MGF_PERIODIC);
            }
            delta = timer_get_ticks() - delta;
            DEBUG_PRINT(("Time(ns): "));
            printk("%llu\n",delta*32);
            //printk("%llu\n",sum);
            count_sample++;
        }
        printk("\n");

        reset(src);
        */
        DEBUG_PRINT(("### PREM TEST ###\n"));
        while(count_sample < SAMPLES){
            sum = 0;
            delta = timer_get_ticks();
            for (j = 0; j < K; j++) {
                gprem_set(PREM_MEMORY,INF,INF,INF,MGF_PERIODIC);
                for (i = 0; i < ARRAY_SIZE; i++) {
                    for(k = 0; k < PART_SIZE; k++) {
                        int index = (j*PART_SIZE) + k;
                        sum += src[index][0];
                        //heartbeat();
                    }
                }
                gprem_set(PREM_COMPUTE,1677,INF,10,MGF_PERIODIC);
            }
            delta = timer_get_ticks() - delta;
            DEBUG_PRINT(("Time(ns): "));
            printk("%llu\n",delta*32);
            //printk("%llu\n",sum);
            count_sample++;
        }
        printk("\n");

    }

    reset(src);
    gprem_set(PREM_COMPUTE,0,0,0,0);
    DEBUG_PRINT(("#### Test ended - halt() ####\n"));
    halt();
}
