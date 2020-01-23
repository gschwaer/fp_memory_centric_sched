/*
 * HERCULES stress inmate
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *  Marco Solieri <ms@xt3.it>
 *
 * N.B: this file should be used for testing only
 */
#include <inmate.h>
#include "gprem.h"
#include "sleep.h"

#define ARRAY_SIZE ((2*1024*1024)/sizeof(int))
#define INF (unsigned long long)(0xffffffffffffffff)
extern void arm_dcaches_flush(void* addr, unsigned int size, int flush);

static void flush_array(int*array)
{
    arm_dcaches_flush(array, ARRAY_SIZE, 0);
}

void inmate_main(void)
{
    int src[ARRAY_SIZE];
    int i,k;
    volatile int sum = 0;

    while(1){
        //printk("Taking lock: %d\n",id);
        gprem_set(PREM_MEMORY,INF,INF,INF,MGF_PERIODIC);
        //printk("Flushing: %d\n",id);
        flush_array(src);
        //printk("RRead: %d\n",id);
        //sleep(30);
        // Random read
        //for (i = 0; i < ARRAY_SIZE; i=(((i*977+32569)%ARRAY_SIZE)%4))
        //    sum &= src[i];
          for (i = 0; i < ARRAY_SIZE; i++)
                sum &= src[(((i*977+32569)%ARRAY_SIZE)%4)];
        //printk("Releasing lock: %d  id:%d\n",sum,id);
        gprem_set(PREM_COMPUTE,1638,INF,100,MGF_PERIODIC);

        for (i = 0; i < ARRAY_SIZE; i++)
            for(k = 0; k < ARRAY_SIZE;k++)
                src[k] += sum;
        //printk("HB: %d\n",id);
		heartbeat();
	}

}
