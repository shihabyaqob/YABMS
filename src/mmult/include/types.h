/* types.h
 *
 * This file contains all required type declarations for the mmult benchmark.
 *
 * Author: Shihab Hasan
 * Date  : 2025-05-12
 *
 * Description:
 *   The structure args_t holds pointers to matrix A, matrix B, and matrix C (result)
 *   along with matrix dimensions M, N, and P. It also includes parameters for CPU
 *   affinity and the number of threads.
 */

#ifndef __INCLUDE_TYPES_H_
#define __INCLUDE_TYPES_H_

typedef struct {
    float* A;      /* Pointer to matrix A (dimensions: M x N) */
    float* B;      /* Pointer to matrix B (dimensions: N x P) */
    float* C;      /* Pointer to result matrix C (dimensions: M x P) */
    int M;         /* Number of rows in matrix A and C */
    int N;         /* Number of columns in A (and rows in B) */
    int P;         /* Number of columns in matrix B and C */
    int cpu;       /* CPU identifier for affinity/scheduling */
    int nthreads;  /* Number of threads available */
    int blocksize;  // <-- Add this for the "opt" implementation (blocked matrix multiplication)
} args_t;

#endif // __INCLUDE_TYPES_H_
