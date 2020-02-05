#include "median.h"

#include "system_config.h"
#include "ee_platform_types.h"

// contains random bytes for the first half of the application data (see EOF)
extern const uint8_t app_data[SIZE_DATA_SECTION];

const uint32_t num_ints = SIZE_DATA_SECTION / sizeof(int);


// function to sort the array in ascending order
void array_sort(int *array , int n)
{ 
    // declare some local variables
    int i=0 , j=0 , temp=0;

    for(i=0 ; i<n ; i++)
    {
        for(j=0 ; j<n-1 ; j++)
        {
            if(array[j]>array[j+1])
            {
                temp        = array[j];
                array[j]    = array[j+1];
                array[j+1]  = temp;
            }
        }
    }
}

static float find_median(int array[] , int n)
{
    float median=0;
    
    // if number of elements are even
    if(n%2 == 0)
        median = (array[(n-1)/2] + array[n/2])/2.0;
    // if number of elements are odd
    else
        median = array[n/2];
    
    return median;
}



volatile int median = 0;
void task_calculate_median( void )
{
	// Sort the array in ascending order
    	array_sort(app_data , num_int);
	median = find_median(app_data, num_int);
}
