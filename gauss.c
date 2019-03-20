/* This file is only for reference. It cannot be compiled successfully, 
 * because m_set_procs(), m_get_numprocs() is not supported. Please 
 * write your own parallel version (Pthread, OpenMP, or MPI verion). For 
 * instance, you should use pthread_create() and pthread_join() to 
 * write a Pthread version, and use MPI initilization and communication
 * functions to write a MPI version.
 */

/* Demostration code - Gaussian elimination without pivoting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <limits.h>
#include <time.h>
// #include <ulocks.h>
// #include <task.h>

/* -------------------------------- Gloobal variables */
/* Program Parameters */
#define MAXN 2000  /* Max value of N */
#define L_cuserid 8
int N;  /* Matrix size */
int procs;  /* Number of processors to use */

/* Matrices and vectors */
volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];
/* A * X = B, solve for X */
pthread_barrier_t row_barrier;

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
      printf("\nSubmission run for \"%s\".\n", cuserid(uid));
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
  printf("\nMatrix dimension N = %i.\n", N);
  printf("Number of processors = %i.\n", procs);

  /* Set number of processors */
  m_set_procs(procs);
}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
  int row, col;

  printf("\nInitializing...\n");
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
  print_inputs();

  /* Start Clock */
  printf("\nStarting clock.\n");
  gettimeofday(&etstart, &tzdummy);
  etstart2 = times(&cputstart);

  /* Gaussian Elimination */
  gauss();

  /* Stop Clock */
  gettimeofday(&etstop, &tzdummy);
  etstop2 = times(&cputstop);
  printf("Stopped clock.\n");
  usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
  usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;

  /* Display output */
  print_X();

  /* Display timing results */
  printf("\nElapsed time = %g ms.\n",
   (float)(usecstop - usecstart)/(float)1000);
  /*printf("               (%g ms according to times())\n",
   *       (etstop2 - etstart2) / (float)CLOCKS_PER_SEC * 1000);
   */
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
      /* Contrary to the man pages, this appears not to include the parent */
  printf("--------------------------------------------\n");

}

/* ------------------ Above Was Provided --------------------- */

/****** You will replace this routine with your own parallel version *******/
/* Provided global variables are MAXN, N, procs, A[][], B[], and X[],
 * defined in the beginning of this code.  X[] is initialized to zeros.
 */

void *eliminate(void *param)
{
    int norm, row, col;  /* Normalization row, and zeroing element row and col */
    float multiplier;
    norm = *((int *) param);

    printf("THREAD RUNNING norm: %d.\n",norm);

    for (row = norm + 1; row < N; row++) {
      multiplier = A[row][norm] / A[norm][norm]; /* Division step */
      for (col = norm; col < N; col++) {
        printf("THREAD RUNNING: [%d,%d].\n",row, col);
        A[row][col] -= A[norm][col] * multiplier; /* Elimination step */
      }
      B[row] -= B[norm] * multiplier;
      // print_inputs();
    }
    // pthread_barrier_wait(&row_barrier);

    pthread_exit(0);
}

void gauss() {
  int norm, row, col;  /* Normalization row, and zeroing element row and col */
  float multiplier;
  // pthread_barrier_t row_barrier, phase_barrier;
  long partial_list_size;
  pthread_t *tids = NULL;
  int i;
  int *index = calloc(procs, sizeof(int));

  /* Initialize thead ids*/
  tids = malloc(sizeof(pthread_t) * procs);
  if (tids == NULL) {
    printf("Error : could not init the tids\n");
    return -1;
  }
  printf("malloc threads\n");

  for (i = 0; i < procs; i++)
  {
    index[i] = i;
  }

  /* Gaussian elimination */
  // for (norm = 0; norm < N - 1; norm++) {

  //   printf("===========%d\n",norm);

  //   // pthread_barrier_init(&row_barrier,NULL,procs+1);

  //   /* create threads */
  //   for (i = 0; i < procs; i++) {
  //     printf("===========procs[%d]\n",i);
  //     if (pthread_create(&tids[i], NULL, &eliminate, &index[i]) != 0) {
  //       printf("Error : pthread_create failed on spawning thread %d\n", i);
  //       return -1;
  //     }
  //   }
  //   // pthread_barrier_wait(&row_barrier);
  //   /* join threads */
  //   for (i = 0; i < procs; i++) {
  //     if (pthread_join(tids[i], &index[i]) != 0) {
  //       printf("Error : pthread_join failed on joining thread %d\n", i);
  //       return -1;
  //     }
  //   }
  //   print_inputs();
  // }

  for (norm = 0; norm < N - 1; norm++) {
    i = norm;
    /* create threads */
    if (pthread_create(&tids[norm], NULL, &eliminate, &index[norm]) != 0) {
        printf("Error : pthread_create failed on spawning thread %d\n", i);
        return -1;
      }
  }

  for (norm = 0; norm < N - 1; norm++) {
    if (pthread_join(tids[norm], &index[norm]) != 0) {
      printf("Error : pthread_join failed on joining thread %d\n", i);
      return -1;
    print_inputs();
  }



  // pthread_barrier_destroy(&row_barrier);

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

  free(tids);
}
