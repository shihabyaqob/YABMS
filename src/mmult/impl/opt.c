/* 
 * impl_opt_mmult.c
 *
 * Optimized (blocked) matrix-matrix multiplication implementation.
 *
 * Author: Shihab Hasan
 * Date  : 2025-04-18
 *
 * Description:
 *   This implementation performs matrix multiplication using a blocking strategy 
 *   to improve cache utilization. Given matrices A (M×N) and B (N×P), the output 
 *   matrix C (M×P) is computed by partitioning the matrices into sub-matrices (blocks)
 *   of size 'b'. For each block, the algorithm accumulates partial products to compute 
 *   the final result. The block size is provided as a command-line argument and passed 
 *   via the args_t structure.
 */

#include <stdlib.h>
#include <stdio.h>
#include "common/macros.h"
#include "common/types.h"
#include "include/types.h"
#include <math.h>



void* impl_mmult_opt(void* args) {
    args_t* parsed_args = (args_t*) args;
    int M = parsed_args->M;
    int N = parsed_args->N;
    int P = parsed_args->P;
    int b = parsed_args->blocksize ;  // Block size for blocked matrix multiplication 

    float* A = parsed_args->A;
    float* B = parsed_args->B;
    float* C = parsed_args->C;

    // Initialize matrix C to zero.
    for (int i = 0; i < M * P; i++) {
        C[i] = 0.0f;
    }

    // Blocked matrix multiplication.
    for (int ii = 0; ii < M; ii += b) {
        for (int jj = 0; jj < P; jj += b) {
            for (int kk = 0; kk < N; kk += b) {
                int i_max = (ii + b > M) ? M : (ii + b);
                int j_max = (jj + b > P) ? P : (jj + b);
                int k_max = (kk + b > N) ? N : (kk + b);
                for (int i = ii; i < i_max; i++) {
                    for (int j = jj; j < j_max; j++) {
                        float sum = C[i * P + j];
                        for (int k = kk; k < k_max; k++) {
                            sum += A[i * N + k] * B[k * P + j];
                        }
                        C[i * P + j] = sum;
                    }
                }
            }
        }
    }
    return NULL;
}