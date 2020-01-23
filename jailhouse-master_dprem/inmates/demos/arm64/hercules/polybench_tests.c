/*
 * HERCULES PolyBench Tests
 *
 * Copyright (C) 2018 Università di Modena e Reggio Emilia
 *
 * Authors:
 *  Luca Miccio <lucmiccio@gmail.com>
 *
 * N.B: this file should be used for testing only
 */

#include "polybench_tests.h"
#include "sleep.h"

/*
 * Tunables
 */
 // Run each poly test for TEST_ITER times
 #define TEST_ITER 10

// Undefine this macro if you want only the "rough" results (ns) without any
// debug output (useful when gathering data for charts)
#define HERCULES_DEBUG

// Print how much time is spent during GPREM hypercalls
// This macro *MUST* be disabled if using non PREM-ized libraries
//#define GPREM_ENABLED

//Print also the entire function duration
//This macro is not evaluated if HERCULES_DEBUG is undefined
#define PRINT_TOTAL_TIME

// Define the number of poly test
#define TEST_SIZE 11

// This array *must* contain $TEST_SIZE element(s)
poly_test tests[] = {
    {
        main_dummy_2mm,
        "2MM",
    },
    {
        main_dummy_2mm,
        "3MM",
    },
    {
        main_dummy_adi,
        "ADI"
    },
    {
        main_dummy_atax,
        "ATAX"
    },
    {
        main_dummy_bicg,
        "BICG"
    },
    {
        main_dummy_covariance,
        "COVARIANCE"
    },
    {
        main_dummy_gemm,
        "GEMM"
    },
    {
        main_dummy_gemver,
        "GEMVER"
    },
    {
        main_dummy_jacobi_2d,
        "JACOBI"
    },
    {
        main_dummy_mvt,
        "MVT"
    },
    {
        main_dummy_seidel,
        "SEIDEL"
    },
};

// DO NOT touch here
#include "utilities.h"

#ifdef GPREM_ENABLED
extern unsigned long long hyp_time;
//extern unsigned long long mem_phases, compatible_phases, compute_phases;
#endif

void test_wrapper(u64(*f_test)(void), char* name)
{
    u64 delta,i;
    u64 kernel_time = 0;

    DEBUG_PRINT(("#### TEST: %s ####\n",name));
    for (i = 0; i < TEST_ITER; i++){
        delta = timer_get_ticks();
        kernel_time = (u64)f_test();
        delta = timer_get_ticks() - delta;
#ifdef PRINT_TOTAL_TIME
        DEBUG_PRINT(("Delta(ticks): %llu -> %llu\n",delta, delta*32));
#endif
        DEBUG_PRINT(("Kernel time(ticks): %llu to ns ",kernel_time));
        printk("%llu ", kernel_time*32);
#ifdef GPREM_ENABLED
        printk("%llu", hyp_time*32);
        //printk("\nM: %lu PAT: %lu PUTE: %lu\n", mem_phases, compatible_phases, compute_phases);
#endif
        printk("\n");
    }
};

void inmate_main(void)
{
    int i = 0;
    DEBUG_PRINT(("\n#### PolyBench test ####\n"));
    DEBUG_PRINT(("Test(s) chosen:\n"));
    printk("\n");
    for (i = 0; i < TEST_SIZE; i++){
        printk("%s ",tests[i].name);
    }
    printk("\n");

    for (i = 0; i < TEST_SIZE; i++){
        test_wrapper(tests[i].f_test, tests[i].name);
        sleep(1);
        printk("\n");
    }

    DEBUG_PRINT(("#### Test ended - halt() ####\n"));
    halt();
}
