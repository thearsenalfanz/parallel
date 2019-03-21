
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
 #include <string.h>

/* -------------------------------- Gloobal variables */
/* Program Parameters */
#define MAXN 2000  /* Max value of N */
#define L_cuserid 8
int N;  /* Matrix size */
int procs;  /* Number of processors to use */
int gnorm;

/* Matrices and vectors */
volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];
/* A * X = B, solve for X */
pthread_barrier_t row_barrier, phase_barrier;

/* junk */
#define randm() 4|2[uid]&3

/* --------------------------------- Prototype */
void gauss();


/* --------------------------------- Functions */
/* returns a seed for srand based on the time */
unsigned int time_seed() {
  struct timeval t;
  struct timezone tzdummy;

  gettimeofday(&t, &tzdummy);
  return (unsigned int)(t.tv_usec);
}

  /* set procs */
void m_set_procs(int p)
{
  procs = p;
}

/* return number of process */
int m_get_numprocs()
{
  return procs;
}

/* Set the program parameters from the command-line arguments */
void parameters(int argc, char **argv) {
  int submit = 0;  /* = 1 if submission parameters should be used */
  int seed = 0;  /* Random seed */
  char uid[L_cuserid + 2]; /*User name */

  /* Read command-line arguments */
  srand(time_seed());  /* Randomize */
  if (argc != 3) {
    if ( argc == 2 && !strcmp(argv[1], "submit") ) {
      /* Use submission parameters */
      submit = 1;
      N = 4;
      procs = 2;
      // printf("\nSubmission run for \"%s\".\n", cuserid(uid));
      srand(randm());
    }
    else {
      if (argc == 4) {
  seed = atoi(argv[3]);
  srand(seed);
  printf("Random seed = %i\n", seed);
      }
      else {
  printf("Usage: %s <matrix_dimension> <num_procs> [random seed]\n",
         argv[0]);
  printf("       %s submit\n", argv[0]);
  exit(0);
      }
    }
  }

  /* Interpret command-line args */
  if (!submit) {
    N = atoi(argv[1]);
    if (N < 1 || N > MAXN) {
      printf("N = %i is out of range.\n", N);
      exit(0);
    }
    procs = atoi(argv[2]);
    if (procs < 1) {
      printf("Warning: Invalid number of processors = %i.  Using 1.\n", procs);
      procs = 1;
    }
    if (procs > m_get_numprocs()) {
      printf("Warning: %i processors requested; only %i available.\n",
       procs, m_get_numprocs());
      procs = m_get_numprocs();
    }
  }

  /* Print parameters */
  // printf("\nMatrix dimension N = %i.\n", N);
  // printf("Number of processors = %i.\n", procs);

  /* Set number of processors */
  m_set_procs(procs);
}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
  int row, col;

  // printf("\nInitializing...\n");
  for (col = 0; col < N; col++) {
    for (row = 0; row < N; row++) {
      A[row][col] = (float)rand() / 32768.0;
    }
    B[col] = (float)rand() / 32768.0;
    X[col] = 0.0;
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
    printf("\nB = [");
    for (col = 0; col < N; col++) {
      printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
    }
  }
}

void print_X() {
  int row;

  if (N < 10) {
    printf("\nX = [");
    for (row = 0; row < N; row++) {
      printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
    }
  }
}

void main(int argc, char **argv) {
  /* Timing variables */
  struct timeval etstart, etstop;  /* Elapsed times using gettimeofday() */
  struct timezone tzdummy;
  clock_t etstart2, etstop2;  /* Elapsed times using times() */
  unsigned long long usecstart, usecstop;
  struct tms cputstart, cputstop;  /* CPU times for my processes */

  /* Process program parameters */
  parameters(argc, argv);

  /* Initialize A and B */
  initialize_inputs();

  /* Print input matrices */
  // print_inputs();

  /* Start Clock */
  // printf("\nStarting clock.\n");
  gettimeofday(&etstart, &tzdummy);
  etstart2 = times(&cputstart);

  /* Gaussian Elimination */
  gauss();

  /* Stop Clock */
  gettimeofday(&etstop, &tzdummy);
  etstop2 = times(&cputstop);
  // printf("Stopped clock.\n");
  usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
  usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;

  /* Display output */
  // print_X();

  /* Display timing results */
  printf("%d,%d,%g\n",N, procs, (float)(usecstop - usecstart)/(float)1000);
  /*printf("               (%g ms according to times())\n", (etstop2 - etstart2) / (float)CLOCKS_PER_SEC * 1000);
   
  printf("(CPU times are accurate to the nearest %g ms)\n",
   1.0/(float)CLOCKS_PER_SEC * 1000.0);
  printf("My total CPU time for parent = %g ms.\n",
   (float)( (cputstop.tms_utime + cputstop.tms_stime) -
      (cputstart.tms_utime + cputstart.tms_stime) ) /
   (float)CLOCKS_PER_SEC * 1000);
  printf("My system CPU time for parent = %g ms.\n",
   (float)(cputstop.tms_stime - cputstart.tms_stime) /
   (float)CLOCKS_PER_SEC * 1000);
  printf("My total CPU time for child processes = %g ms.\n",
   (float)( (cputstop.tms_cutime + cputstop.tms_cstime) -
      (cputstart.tms_cutime + cputstart.tms_cstime) ) /
   (float)CLOCKS_PER_SEC * 1000);
  printf("--------------------------------------------\n");
  */

}

/* thread function*/
void *eliminate(void *param)
{
    int norm, row, col;  /* Normalization row, and zeroing element row and col */
    float multiplier; /* multiplier */
    int i = *((int *) param); /* thread index */
    norm = gnorm;

    /* thread running for assigned rows */
    for (row = norm+1+i; row < N; row+=procs) {
      // printf("[%d] ROW %d\n",i, row);
      multiplier = A[row][norm] / A[norm][norm]; /* Division step */
      for (col = norm; col < N; col++) {
        // printf("[%d] CELL [%d,%d].\n",i, row, col);
        A[row][col] -= A[norm][col] * multiplier; /* Elimination step */
      }
      B[row] -= B[norm] * multiplier;
    }

    /* increment barrier wait */
    pthread_barrier_wait(&row_barrier);
    pthread_exit(0);
}


void gauss() {
  int norm, row, col;  /* Normalization row, and zeroing element row and col */
  pthread_t *tids = NULL; /* thread id*/
  int i,t;
  int *index = calloc(procs, sizeof(int)); /* thread index for row assignment for each thread */

  /* Initialize thread ids*/
  tids = malloc(sizeof(pthread_t) * procs);
  if (tids == NULL) {
    printf("Error : could not init the tids\n");
  }

  /* initialize barrier */
  pthread_barrier_init(&row_barrier,NULL,procs+1);

  /* Gaussian elimination */
  for (norm = 0; norm < N-1; norm++) {
    gnorm = norm;

    /* create threads */
    for (t = 0; t < procs; t++) {
      index[t] = t;
      // printf("INDEX %d = %d\n",t,index[t] );
      if (pthread_create(&tids[t], NULL, &eliminate, &index[t]) != 0) {
        printf("Error : pthread_create failed on spawning thread %d\n", t);
      }
    }

    /* wait until all threads are done*/
    pthread_barrier_wait(&row_barrier);

    /* join threads */
    for (t = 0; t < procs; t++) {
      if (pthread_join(tids[t], &index[t]) != 0) {
        printf("Error : pthread_join failed on joining thread %d\n", t);
      }
    }

  }

  /* (Diagonal elements are not normalized to 1.  This is treated in back
   * substitution.)
   */
  /* Back substitution */
  for (row = N - 1; row >= 0; row--) {
    X[row] = B[row];
    for (col = N-1; col > row; col--) {
      X[row] -= A[row][col] * X[col];
    }
    X[row] /= A[row][row];
  }
  free(index);
  free(tids);
}
