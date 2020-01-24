#include "average.h"

#include "system_config.h"
#include "ee_platform_types.h"

// contains random bytes for the first half of the application data (see EOF)
extern const uint8_t app_data[SIZE_DATA_SECTION];

const uint32_t num_ints = SIZE_DATA_SECTION / sizeof(int);

static int calc_avg(int *data) {
	long long avg = 0;

	for (unsigned int i = 0; i < num_ints; ++i) {
		avg += data[i];
	}

	return avg / num_ints;
}

volatile int avg = 0;
void task_calculate_average( void )
{
	avg = calc_avg((int*)app_data);
}
