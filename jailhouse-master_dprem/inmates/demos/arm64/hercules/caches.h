/*
 * Cache functionalities
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */

 void flush_i_cache(void);
 void prefetch(void* address);

 void flush_i_cache(void){
     asm volatile ("ic iallu");
     asm volatile ("dsb sy" : : : "memory");
     asm volatile ("dsb ish" : : : "memory");
 }

 void prefetch(void* address)
 {
     // Prefetch in memory
     // For load, in L2 and keep in cache
     asm volatile ("prfm pldl2keep, %a0\n" : : "p" (address));
 }
