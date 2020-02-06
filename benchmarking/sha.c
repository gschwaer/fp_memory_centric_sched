#include "sha.h"

#include "system_config.h"
#include "ee_platform_types.h"

#include "sha/sha.h"

// contains random bytes for the first half of the application data (see EOF)
extern const uint8_t app_data[SIZE_DATA_SECTION];

//static const uint32_t num_ints = SIZE_DATA_SECTION / sizeof(int);

volatile int sha = 0;
void benchmark_calculate_sha( void )
{
	sha_init();
	sha_main(app_data, 32743);
	sha = sha_return();
}
