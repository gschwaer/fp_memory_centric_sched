#include "memory_prefetching_inc.h"


.global clear_cache_partition_L2
.type clear_cache_partition_L2, %function
.section .text
.func
clear_cache_partition_L2:
#define StartWay x0
#define EndWay x1
#define StartSet x2
#define EndSet x3
#define Level  x9
#define SetWay x10
#define Ways   x11
#define Sets   x12

	MOV   Level, #LEVEL
	MOV   Ways, EndWay
clear_cache_partition_L2_loop_reset_sets:
	MOV   Sets, EndSet
clear_cache_partition_L2_loop:
	ADD   SetWay, Level, Ways, LSL #WAYS_OFF  // create new bitfield: level and ways
	ADD   SetWay, SetWay, Sets, LSL #SETS_OFF // create new bitfield: add sets
	DC    CISW, SetWay                        // clear invalidate set way
	CMP   Sets, StartSet
	Bls   clear_cache_partition_L2_dec_way    // if Sets == 0: all the sets cleared -> next way
	SUB   Sets, Sets, #1                      // next set
	B     clear_cache_partition_L2_loop
clear_cache_partition_L2_dec_way:
	CMP   Ways, StartWay
	Bls   clear_cache_partition_L2_end_loop   // ways are 0 so we are done
	SUB   Ways, Ways, #1                      // next way
	B     clear_cache_partition_L2_loop_reset_sets
clear_cache_partition_L2_end_loop:

	/* flush pipeline */
	dsb   SY
	isb
	ret

#undef StartWay
#undef EndWay
#undef StartSet
#undef EndSet
#undef Level
#undef SetWay
#undef Ways
#undef Sets
.endfunc


.global prefetch_memory
.type prefetch_memory, %function
.section .text
.func
prefetch_memory:
#define Address x0
#define NumLines x1

prefetch_memory_loop:
	prfm  PLDL2KEEP, [Address]
	add   Address, Address, #CACHE_LINE_SIZE
	subs  NumLines, NumLines, #1
	beq   prefetch_memory_end
	b     prefetch_memory_loop
prefetch_memory_end:

	/* flush pipeline */
	dsb   SY
	isb
	ret

#undef Address
#undef NumLines
.endfunc
