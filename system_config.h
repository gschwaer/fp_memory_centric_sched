#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

#include "config.h"

// cache infos
#define CACHE_LINE_SIZE 64 /* byte */
#define L2_SIZE 0x200000
#define L2_WAYS 16

// cpu infos
#define NUM_CPUS 4

/* calculated */
#define CACHE_LINE_SIZE_LOG_2 LOG2(CACHE_LINE_SIZE)
#define L2_LINES (L2_SIZE/CACHE_LINE_SIZE)
#define L2_SETS (L2_LINES/L2_WAYS)
#define L2_BYTES_PER_SET (L2_WAYS*CACHE_LINE_SIZE)
#define L2_FIRST_WAY 0
#define L2_LAST_WAY (L2_WAYS-1)
#define L2_FIRST_SET 0
#define L2_LAST_SET (L2_SETS-1)
#define PARTITION_FIRST_SET ((L2_SETS / NUM_CPUS) * CACHE_PARTITION)
#define PARTITION_LAST_SET (((L2_SETS / NUM_CPUS) * (CACHE_PARTITION + 1)) - 1)

#endif /* __SYSTEM_CONFIG_H__ */
