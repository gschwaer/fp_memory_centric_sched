#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

// Compile with gcc <filename>

#define NUM_INTS 458752
#define NEWLINE_EVERY 1000

int main( void )
{
	fprintf( stderr, "Generating %u random uint8_t:\n", NUM_INTS);
	assert(RAND_MAX == 0x7FFFFFFF);
	assert(sizeof( unsigned int ) == 4);

	srand( 0xABABCDCD );

	for ( unsigned int i = 0; i < NUM_INTS; ++i ) {
		uint8_t random_uint = ( uint8_t )((( unsigned int ) rand() ) % 0x0100 );
		printf( "%u,", random_uint );
		if(( i % NEWLINE_EVERY ) == ( NEWLINE_EVERY - 1 ))
			printf( "\n" );
	}
	printf( "\n" );

	exit( 0 );
}
