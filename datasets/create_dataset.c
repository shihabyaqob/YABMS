/* create_dataset.c
 *
 * Generates a dataset file for the mmult benchmark.
 *
 * Author: Shihab Hasan
 * Date  : 2025-05-12
 *
 * Description:
 * Generate a file that contains A, B, and R (the golden reference) for a
 * given M, N, P.  All stored in row-major order, single-precision float.
 *
 * Usage:  
 * First compile the program:
 *  gcc -o create_dataset create_dataset.c
 * 
 * Second, run the program with the following arguments:
 * ./create_dataset <filename> <M> <N> <P>
 *
 * Example:
 *   ./create_dataset testing.bin 16 12 8
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*

 */

int main(int argc, char** argv) {
    if (argc < 5) {
        printf("Usage: %s <filename> <M> <N> <P>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    int M = atoi(argv[2]);
    int N = atoi(argv[3]);
    int P = atoi(argv[4]);

    // Seed RNG for reproducibility, or use time(NULL).
    srand(0);

    // Allocate A, B, R
    size_t sizeA = M * N;
    size_t sizeB = N * P;
    size_t sizeC = M * P;

    float* A = (float*)malloc(sizeA * sizeof(float));
    float* B = (float*)malloc(sizeB * sizeof(float));
    float* R = (float*)malloc(sizeC * sizeof(float));
    if (!A || !B || !R) {
        printf("Error: memory allocation failed.\n");
        return 1;
    }

    // Generate random A, B in [0..1), compute R = A*B
    for (size_t i = 0; i < sizeA; i++) {
        A[i] = (float)rand() / (float)RAND_MAX;
    }
    for (size_t i = 0; i < sizeB; i++) {
        B[i] = (float)rand() / (float)RAND_MAX;
    }

    // Compute reference R
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            R[i * P + j] = 0.0f;
            for (int k = 0; k < N; k++) {
                R[i * P + j] += A[i * N + k] * B[k * P + j];
            }
        }
    }

    // Write A, B, and R to file in row-major order
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("Error: cannot open file %s\n", filename);
        return 1;
    }

    fwrite(A, sizeof(float), sizeA, fp);
    fwrite(B, sizeof(float), sizeB, fp);
    fwrite(R, sizeof(float), sizeC, fp);

    fclose(fp);
    free(A);
    free(B);
    free(R);

    printf("Generated dataset %s with dimensions M=%d, N=%d, P=%d\n", filename, M, N, P);
    return 0;
}
