#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/time.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <mpi.h>

#define TAG 13

/* Program Parameters */
#define MAXN 2000  /* Max value of N */
#define L_cuserid 8
int N;  /* Matrix size */
int procs;  /* Number of processors to use */

/* A * X = B, solve for X */

/* junk */
#define randm() 4|2[uid]&3

/* Prototype */
void gauss();  /* The function you will provide.
    * It is this routine that is timed.
    * It is called only on the parent.
    */

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
void parameters(int argc, char **argv, int myrank) {
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
  if(myrank == 0)
  {
    printf("\nMatrix dimension N = %i.\n", N);
    printf("Number of processors = %i.\n", procs);
  }

  /* Set number of processors */
  m_set_procs(procs);
}

// /* Initialize A and B (and X to 0.0s) */
// void initialize_inputs(float **A, float *B) {
//   int row, col;

//   printf("\nInitializing...\n");

//   for (col = 0; col < N; col++) {
//     for (row = 0; row < N; row++) {
//       A[row][col] = (float)rand() / 32768.0;
//     }
//     B[col] = (float)rand() / 32768.0;
//     X[col] = 0.0;
//   }

// }

/* Print input matrices */
void print_inputs(float *A, float *B) {
  int row, col;

  if (N < 10) {
    printf("\nA =\n\t");
    for (row = 0; row < N; row++) {
      for (col = 0; col < N; col++) {
  printf("%5.2f%s", A[col+N*row], (col < N-1) ? ", " : ";\n\t");
      }
    }
    printf("\nB = [");
    for (col = 0; col < N; col++) {
      printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
    }
  }
}

void print_X(float *X) {
  int row;

  if (N < 10) {
    printf("\nX = [");
    for (row = 0; row < N; row++) {
      printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
    }
  }
}

void print(float *Y, char *name, int size) {
  int row;

  if (N < size) {
    printf("\n%s = [", name);
    for (row = 0; row < N; row++) {
      printf("%5.2f%s", Y[row], (row < N-1) ? "; " : "]\n");
    }
  }
}

int main(int argc, char **argv) {
  /* Timing variables */
  // struct timeval etstart, etstop;  /* Elapsed times using gettimeofday() */
  // struct timezone tzdummy;
  // clock_t etstart2, etstop2;  /* Elapsed times using times() */
  // unsigned long long usecstart, usecstop;
  // struct tms cputstart, cputstop;  /* CPU times for my processes */
  
  /* Matrices and vectors */
  float *A, *B, *C, *X;
  double startTime, endTime;
  int myrank, numnodes;

  int norm, row, col;  /* Normalization row, and zeroing  element row and col */
  float multiplier; /*multiplier*/
  int *map;
  int i;

  /* Initialize MPI*/
  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &numnodes);

  //////////////////////////////////////////////

  /* Process program parameters */
  parameters(argc, argv, myrank);

  A = (float*)malloc(N*N*sizeof(float));
  B = (float*)malloc(N*sizeof(float));
  C = (float*)malloc(N*sizeof(float));
  X = (float*)malloc(N*sizeof(float));


  if(myrank==0)
  {

    /* Initialize A and B */
    for (col = 0; col < N; col++) {
      for (row = 0; row < N; row++) {
        A[col+N*row] = (float)rand() / 32768.0;
      }
      B[col] = (float)rand() / 32768.0;
      X[col] = 0.0;
    }

    /* Print input matrices */
    print_inputs(A,B);
  }

  MPI_Barrier(MPI_COMM_WORLD);


  MPI_Bcast (&A[0],N*N,MPI_FLOAT,0,MPI_COMM_WORLD);
  MPI_Bcast (B,N,MPI_FLOAT,0,MPI_COMM_WORLD);

  map = calloc(N,sizeof(int));

  for(i=0; i<N; i++)
  {
    map[i]= i % procs;
    // printf("[proc %d] %d, %d\n",myrank, i, map[i]);
  } 

  printf("[proc %d] N = %d\n",myrank, N); 

  MPI_Barrier(MPI_COMM_WORLD);
  if(myrank == 0)
  {
    /* Start Clock */
    printf("\nStarting clock.\n");
    // gettimeofday(&etstart, &tzdummy);
    // etstart2 = times(&cputstart);
    startTime = MPI_Wtime();
  }
  MPI_Barrier(MPI_COMM_WORLD);

  printf("hello world from %d\n", myrank);

  /////////////////////////////////////////

  /* Gaussian elimination */
  for(norm = 0; norm < N; norm++)
  {
    /* parallelize */
    MPI_Bcast (&A[norm][norm], N-norm, MPI_FLOAT, map[norm], MPI_COMM_WORLD);
    MPI_Bcast (&B[norm], 1, MPI_FLOAT, map[norm], MPI_COMM_WORLD);
    for(row = norm+1; row < N; row++) 
    {
      if(map[row] == myrank)
      {
        C[row] = A[row][norm]/A[norm][norm];
      }
    }        
    for(row = norm+1; row < N; row++) 
    { 
      if(map[row] == myrank)
      {
        for(col = 0;col < N;col++)
        {
          A[row][col] = A[row][col]-( C[row]*A[norm][col] );
        }
        B[row] = B[row]-( C[row]*B[norm] );
      }
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  
   (Diagonal elements are not normalized to 1.  This is treated in back
   * substitution.)
   
  /* Back substitution */
  if(myrank == 0)
  {
    for (row = N - 1; row >= 0; row--) {
      X[row] = B[row];
      for (col = N-1; col > row; col--) {
        X[row] -= A[row][col] * X[col];
      }
      X[row] /= A[row][row];
    }
  }

  //////////////////////////////////////////

  if(myrank==0)
  {
    /* Stop Clock */
    // gettimeofday(&etstop, &tzdummy);
    // etstop2 = times(&cputstop);
    // printf("Stopped clock.\n");
    // usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
    // usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;
    endTime = MPI_Wtime();
    
    /* Display output */
    print_X(X);

    
    printf("Elapsed time %f\n", endTime-startTime);
    printf("--------------------------------------------\n");
  }

  MPI_Finalize();
  return 0;

}

