#ifndef POLYBENCH_TESTS_H
#define POLYBENCH_TESTS_H

#include <inmate.h>
#include <poly_2mm.h>
#include <poly_3mm.h>
#include <poly_adi.h>
#include <poly_atax.h>
#include <poly_bicg.h>
#include <poly_covariance.h>
#include <poly_gemm.h>
#include <poly_gemver.h>
#include <poly_jacobi_2d.h>
#include <poly_mvt.h>
#include <poly_seidel.h>

typedef struct
{
    u64(*f_test)(void);
    char* name;
}poly_test;

void test_wrapper(u64(*f_test)(void), char* name);

#endif /* !POLYBENCH_TESTS_H */
