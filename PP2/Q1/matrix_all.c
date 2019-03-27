#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>


/* -----------------------------------Global Vars */
#define MAXN 1000
int nthreads, N, seed;
volatile float A[MAXN][MAXN], B[MAXN][MAXN], C[MAXN][MAXN];

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
    int row, col;
    srand(seed);

    printf("\nInitializing inputs\n");
    for (col = 0; col < N; col++) {
        for (row = 0; row < N; row++) {
            A[row][col] = (float)rand() / 32768.0;
            B[row][col] = (float)rand() / 32768.0;
        }
    }
}


/* Print input matrices */
void print_inputs() {
    int row, col;

    if (N < 10) {
        printf("\nA =\n\t");
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                printf("%5.2f%s", A[row][col], (col < N-1) ? ", " : ";\n\t");
            }
        }
        printf("\nB =\n\t");
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                printf("%5.2f%s", B[row][col], (col < N-1) ? ", " : ";\n\t");
            }
        }
        printf("\n");
    }
}

void print_output() {
    int row, col;

    if (N < 10) {
        printf("\nC =\n\t");
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                printf("%5.2f%s", C[row][col], (col < N-1) ? ", " : ";\n\t");
            }
        }
        printf("\n");
    }
}

/* ---------------------------------------- MYSECOND */
static double 
gettime()
{
	struct timeval	tp;
	struct timezone	tzp;
	int i = 0;

	i = gettimeofday(&tp, &tzp);
	return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

int main(int argc, char **argv)
{
    int i,j,k;
    int tid;
    int ret = 0;
    double start,end;

    /* set default*/
    nthreads = 4;
    N = 4;
    seed = 0;

    /* print usage */
    printf("Usage : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
    /* parse the command line args */
    while ((ret = getopt(argc, argv, "S:T:N:h")) != -1) {
        switch (ret) {
        case 'h':
            printf("Usage : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
            return 0;
            break;
        case 'S':
            seed = atoi(optarg);
            break;
        case 'N':
            N = atol(optarg);
            break;
        case 'T':
            nthreads = atoi(optarg);
            break;
        case '?':
        default:
            printf("Unknown option : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
            return -1;
            break;
        }
    }

    initialize_inputs();

    print_inputs();

    printf("Number of threads = %d\n", nthreads);

    start = gettime();
    #pragma omp parallel private(tid) shared (A, B, C, N) num_threads(nthreads)
    #pragma omp for schedule(static)
    for (i = 0; i < N; i++) {
        // tid = omp_get_thread_num();
        // printf("from thread = %d\n", tid);
        #pragma omp parallel private(tid) shared (A, B, C, N) num_threads(nthreads)
        #pragma omp for schedule(static)
        for (j = 0; j < N; j++) {
            C[i][j] = 0;
            #pragma omp parallel private(tid) shared (A, B, C, N) num_threads(nthreads)
            #pragma omp for schedule(static)
            for (k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    end = gettime();

    print_output();
    printf("Runtime of %d threads = %f seconds\n", nthreads, (end-start));
    printf("\n");


    exit(0);
}
