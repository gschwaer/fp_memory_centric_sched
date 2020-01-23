#ifndef __MEMORY_PHASE_FUNCTIONS_H__
#define __MEMORY_PHASE_FUNCTIONS_H__

#include "ee_platform_types.h"

void clear_cache_partition_L2(uint64_t first_way, uint64_t last_way, uint64_t first_set, uint64_t last_set);
void prefetch_memory(uint64_t address, uint64_t num_lines);

#endif /* __MEMORY_PHASE_FUNCTIONS_H__ */
