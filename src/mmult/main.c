/* main.c
 *
 * Main driver for the mmult benchmark.
 *
 * Author: Shihab Hasan
 * Date  : 2025-05-12
 *
 * Description:
 *   This program parses command–line arguments to select the implementation
 *   (naive in this case) and matrix dimensions (M, N, P). It then allocates and
 *   initializes matrices A and B with random floating–point values, allocates
 *   matrix C for the result, and computes a golden reference using a simple
 *   reference computation. The chosen implementation is then run multiple times,
 *   timing each execution and finally verifying the result.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include "common/macros.h"
#include "common/types.h"
#include "include/types.h"
#include "impl/naive.h"

/* Default matrix dimensions (can be overridden via command-line) */
#define DEFAULT_M 16
#define DEFAULT_N 12
#define DEFAULT_P 8

int main(int argc, char** argv)
{
    /* Set stdout buffering */
    setbuf(stdout, NULL);

    /* Default parameters */
    int M = DEFAULT_M;
    int N = DEFAULT_N;
    int P = DEFAULT_P;
    int nruns = 1000;
    int nstdevs = 3;
    int nthreads = 1;
    int cpu = 0;

    /* Function pointer for implementation and corresponding string */
    void* (*impl_ptr)(void* args) = NULL;
    const char* impl_str = NULL;

    bool help = false;

    /* Argument Parsing */
    for (int i = 1; i < argc; i++) {
        /* Implementation selection */
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {
            assert(++i < argc);
            if (strcmp(argv[i], "naive") == 0) {
                impl_ptr = impl_scalar_naive;
                impl_str = "scalar_naive";
            } else {
                impl_ptr = NULL;
                impl_str = "unknown";
            }
            continue;
        }
        /* Matrix dimension: M */
        if (strcmp(argv[i], "-M") == 0) {
            assert(++i < argc);
            M = atoi(argv[i]);
            continue;
        }
        /* Matrix dimension: N */
        if (strcmp(argv[i], "-N") == 0) {
            assert(++i < argc);
            N = atoi(argv[i]);
            continue;
        }
        /* Matrix dimension: P */
        if (strcmp(argv[i], "-P") == 0) {
            assert(++i < argc);
            P = atoi(argv[i]);
            continue;
        }
        /* Number of runs */
        if (strcmp(argv[i], "--nruns") == 0) {
            assert(++i < argc);
            nruns = atoi(argv[i]);
            continue;
        }
        /* Standard deviation threshold */
        if (strcmp(argv[i], "--nstdevs") == 0) {
            assert(++i < argc);
            nstdevs = atoi(argv[i]);
            continue;
        }
        /* Number of threads */
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nthreads") == 0) {
            assert(++i < argc);
            nthreads = atoi(argv[i]);
            continue;
        }
        /* CPU selection */
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cpu") == 0) {
            assert(++i < argc);
            cpu = atoi(argv[i]);
            continue;
        }
        /* Help */
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            help = true;
            continue;
        }
    }

    if (help || impl_ptr == NULL) {
        if (!help) {
            if (impl_str)
                printf("\nERROR: Unknown \"%s\" implementation.\n", impl_str);
            else
                printf("\nERROR: No implementation was chosen.\n");
        }
        printf("\nUsage:\n");
        printf("  %s -i impl -M <rows> -N <inner> -P <cols> [Options]\n", argv[0]);
        printf("\nRequired:\n");
        printf("  -i | --impl      Available implementations: {naive}\n");
        printf("  -M               Number of rows in matrix A\n");
        printf("  -N               Number of columns in matrix A (and rows in matrix B)\n");
        printf("  -P               Number of columns in matrix B\n");
        printf("\nOptions:\n");
        printf("  -h | --help      Print this message\n");
        printf("  -n | --nthreads  Set number of threads (default = %d)\n", nthreads);
        printf("  -c | --cpu       Set the main CPU (default = %d)\n", cpu);
        printf("  --nruns          Number of runs (default = %d)\n", nruns);
        printf("  --nstdevs        Standard deviation threshold (default = %d)\n", nstdevs);
        printf("\n");
        exit(help ? 0 : 1);
    }

    /* Print configuration */
    printf("Running \"%s\" implementation of mmult\n", impl_str);
    printf("Matrix dimensions: A(%d x %d), B(%d x %d), C(%d x %d)\n", M, N, N, P, M, P);
    printf("Number of runs: %d\n", nruns);

    /* Set up scheduling and affinity */
    int nice_level = -20;
    printf("Setting up schedulers and affinity:\n");
    printf("  * Setting the niceness level:\n");
    do {
        errno = 0;
        printf("      -> trying niceness level = %d\n", nice_level);
        (void)nice(nice_level);
    } while (errno != 0 && nice_level++);
    printf("    + Process has niceness level = %d\n", nice_level);

    #if !defined(__APPLE__)
    printf("  * Setting up FIFO scheduling scheme and high priority ... ");
    pid_t pid = 0;
    int policy = SCHED_FIFO;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(policy);
    int res = sched_setscheduler(pid, policy, &param);
    if (res != 0) {
        printf("Failed\n");
    } else {
        printf("Succeeded\n");
    }
    printf("  * Setting up scheduling affinity ... ");
    cpu_set_t cpumask;
    CPU_ZERO(&cpumask);
    for (int i = 0; i < nthreads; i++) {
        CPU_SET((cpu + i) % nthreads, &cpumask);
    }
    res = sched_setaffinity(pid, sizeof(cpumask), &cpumask);
    if (res != 0) {
        printf("Failed\n");
    } else {
        printf("Succeeded\n");
    }
    #endif
    printf("\n");

    /* Initialize statistics */
    __DECLARE_STATS(nruns, nstdevs);

    /* Data allocation and initialization */
    int sizeA = M * N;
    int sizeB = N * P;
    int sizeC = M * P;

    /* Allocate matrices A, B, and C */
    float* A = __ALLOC_DATA(float, sizeA);
    float* B = __ALLOC_DATA(float, sizeB);
    float* C = __ALLOC_DATA(float, sizeC);

    /* Initialize matrices A and B with random floating–point values (0 to 1) */
    for (int i = 0; i < sizeA; i++) {
        A[i] = (float)rand() / (float)RAND_MAX;
    }
    for (int i = 0; i < sizeB; i++) {
        B[i] = (float)rand() / (float)RAND_MAX;
    }

    /* Compute the golden reference using a simple reference mmult */
    float* ref = __ALLOC_DATA(float, sizeC);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            ref[i * P + j] = 0.0f;
            for (int k = 0; k < N; k++) {
                ref[i * P + j] += A[i * N + k] * B[k * P + j];
            }
        }
    }

    /* Set guard bytes at the end of matrix C to check for buffer overruns */
    __SET_GUARD(C, sizeC * sizeof(float));

    /* Prepare the arguments for the implementation */
    args_t args;
    args.A = A;
    args.B = B;
    args.C = C;
    args.M = M;
    args.N = N;
    args.P = P;
    args.cpu = cpu;
    args.nthreads = nthreads;

    /* Run the chosen implementation repeatedly */
    printf("Running \"%s\" implementation:\n", impl_str);
    printf("  * Invoking the implementation %d times .... ", num_runs);
    for (int i = 0; i < num_runs; i++) {
        __SET_START_TIME();
        (*impl_ptr)(&args);
        __SET_END_TIME();
        runtimes[i] = __CALC_RUNTIME();
    }
    printf("Finished\n");

    /* Verification: use floating-point tolerant check */
    printf("  * Verifying results .... ");
    bool match = __CHECK_FLOAT_MATCH(ref, C, sizeC, 1e-6);
    bool guard = __CHECK_GUARD(C, sizeC * sizeof(float));
    if (match && guard) {
        printf("Success\n");
    } else if (!match && guard) {
        printf("Fail, but no buffer overruns\n");
    } else if (match && !guard) {
        printf("Success, but failed buffer overruns check\n");
    } else {
        printf("Failed, and failed buffer overruns check\n");
    }

    /* Compute runtime statistics with outlier filtering */
    uint64_t min = -1;
    uint64_t max = 0;
    uint64_t avg = 0;
    uint64_t avg_n = 0;
    uint64_t std = 0;
    uint64_t std_n = 0;
    int n_msked = 0;
    int n_stats = 0;
    for (int i = 0; i < num_runs; i++)
        runtimes_mask[i] = true;
    printf("  * Running statistics:\n");
    do {
        n_stats++;
        printf("    + Starting statistics run number #%d:\n", n_stats);
        avg_n = 0;
        avg = 0;
        min = -1;
        max = 0;
        for (int i = 0; i < num_runs; i++) {
            if (runtimes_mask[i]) {
                if (runtimes[i] < min)
                    min = runtimes[i];
                if (runtimes[i] > max)
                    max = runtimes[i];
                avg += runtimes[i];
                avg_n++;
            }
        }
        avg = avg / avg_n;
        std = 0;
        std_n = 0;
        for (int i = 0; i < num_runs; i++) {
            if (runtimes_mask[i]) {
                std += ((runtimes[i] - avg) * (runtimes[i] - avg));
                std_n++;
            }
        }
        std = sqrt(std / std_n);
        n_msked = 0;
        for (int i = 0; i < num_runs; i++) {
            if (runtimes_mask[i]) {
                if (runtimes[i] > avg) {
                    if ((runtimes[i] - avg) > (nstd * std)) {
                        runtimes_mask[i] = false;
                        n_msked++;
                    }
                } else {
                    if ((avg - runtimes[i]) > (nstd * std)) {
                        runtimes_mask[i] = false;
                        n_msked++;
                    }
                }
            }
        }
        printf("      - Standard deviation = %" PRIu64 "\n", std);
        printf("      - Average = %" PRIu64 "\n", avg);
        printf("      - Number of active elements = %" PRIu64 "\n", avg_n);
        printf("      - Number of masked-off = %d\n", n_msked);
    } while (n_msked > 0);
    printf("  * Runtimes (%s): %" PRIu64 " ns\n", __PRINT_MATCH(match), avg);


    printf("  * Dumping runtime informations:\n");
    FILE* fp;
    char filename[256];
    strcpy(filename, impl_str);
    strcat(filename, "_runtimes.csv");
    printf("    - Filename: %s\n", filename);
    printf("    - Opening file .... ");
    fp = fopen(filename, "w");
    if (fp != NULL) {
        printf("Succeeded\n");
    
        /* Write CSV headers */
        fprintf(fp, "impl,%s\n", impl_str);
        fprintf(fp, "num_of_runs,%d\n", num_runs);
    
        /* Write all runtimes */
        fprintf(fp, "runtimes");
        for (int i = 0; i < num_runs; i++) {
            fprintf(fp, ", %" PRIu64, runtimes[i]);
        }
        fprintf(fp, "\n");
    
        /* Write average */
        fprintf(fp, "avg,%" PRIu64, avg);
    
        /* Add missing messages here */
        printf("    - Writing runtimes ... Finished\n");
        printf("    - Closing file handle .... ");
        fclose(fp);
        printf("Finished\n");
    
    } else {
        printf("Failed\n");
    }
    printf("\n");
    

    /* Free allocated memory */
    free(A);
    free(B);
    free(C);
    free(ref);

    __DESTROY_STATS();

    return 0;
}