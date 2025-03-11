/* naive.c
 *
 * Naïve (scalar) matrix–matrix multiplication implementation.
 *
 * Author: Shihab Hasan
 * Date  : 2025-05-12
 *
 * Description:
 *   This implementation multiplies matrix A (dimensions M×N) with matrix B (dimensions N×P)
 *   to produce the result matrix C (dimensions M×P). Compiler optimization pragmas are used
 *   to mimic the behavior in the vvadd benchmark.
 */

/* Standard C includes */
#include <stdlib.h>
#include <stdio.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

/* Naïve Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")

void* impl_scalar_naive(void* args)
{
    /* Get the argument structure */
    args_t* parsed_args = (args_t*) args;

    /* Extract matrix dimensions */
    int M = parsed_args->M;  /* Rows in matrix A and C */
    int N = parsed_args->N;  /* Columns in matrix A and rows in matrix B */
    int P = parsed_args->P;  /* Columns in matrix B and C */

    /* Get matrix pointers */
    float* A = parsed_args->A;
    float* B = parsed_args->B;
    float* C = parsed_args->C;

    /* Perform matrix multiplication:
       For each element C[i][j], compute:
         C[i][j] = sum over k of (A[i][k] * B[k][j])
       (Assuming row–major storage)
    */

    // Algorithm 1 Naive mmult Implementation
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            C[i * P + j] = 0.0f;
            for (int k = 0; k < N; k++) {
                C[i * P + j] += A[i * N + k] * B[k * P + j];
            }
        }
    }

    return NULL;
}
#pragma GCC pop_options
