/*
 * HERCULES stress inmate
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */
#include <inmate.h>

#define ARRAY_SIZE ((4*1024*1024)/sizeof(int))

void inmate_main(void)
{
    int src[ARRAY_SIZE];
    int dst[ARRAY_SIZE];

    /* Dumb memory stress */
	while(1){
        memcpy(&src,&dst,ARRAY_SIZE);
		heartbeat();
	}
}
