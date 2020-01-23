/*
 * HERCULES stress inmate
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Marco Solieri <ms@xt3.it>
 *
 * N.B: this file should be used for testing only
 */
#include <inmate.h>
#include "gprem.h"

#define ARRAY_SIZE ((4*1024*1024)/sizeof(int))

void inmate_main(void)
{
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];
    memguard_call(
        //1000, //    1 ms
        //1638, // ~100 KiB --> 100 MiB/s
        100,    //  100 us
        16,     //   ~1 KiB  --> 10 MiB/s
        1);
	while(1){
        memcpy(&src,&dst,ARRAY_SIZE);
		heartbeat();
	}
    memguard_call(0,0,0);

}
