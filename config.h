#ifndef __CONFIG_H__
#define __CONFIG_H__

// the maximum data size for each benchmark
#define SIZE_DATA_SECTION (448*1024) /* this leaves 64k for text and Erika */
#define PARTITION_SIZE (512*1024)

// the cache partition of this application (0 to NUM_CPUS-1)
#define CACHE_PARTITION 0

#endif /* __CONFIG_H__ */
