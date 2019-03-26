#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <sys/time.h>

/* -----------------------------------Global Vars */
#define MAXN 100
int tid, nthreads, N;
volatile float A[MAXN][MAXN], B[MAXN][MAXN], C[MAXN][MAXN];

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
    int row, col;

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
    double start,end;

    nthreads = 4;
    N = 4;

    initialize_inputs();

    print_inputs();

    start = gettime();

    #pragma omp parallel private(tid) shared (A, B, C, N) num_threads(nthreads)
    #pragma omp for schedule(static)
    for (i = 0; i < N; i++) {
        tid = omp_get_thread_num();
        printf("Hello World from thread = %d\n", tid);
        for (j = 0; j < N; j++) {
            C[i][j] = 0;
            for (k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    end = gettime();

    print_output();
    printf("\n");
    printf("Runtime of %d threads = %f seconds\n", nthreads, (end-start));
    printf("\n");


    exit(0);
}
