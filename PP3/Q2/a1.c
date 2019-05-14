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
#define MAXN 2000 /* Max value of N */
#define L_cuserid 8
int N;  /* Matrix size */
int procs;  /* Number of processors to use */

/* Matrices and vectors */
float *A, *B, *X;

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
        // printf("Random seed = %i\n", seed);
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
    printf("Random seed = %i\n", seed);
    printf("\nMatrix dimension N = %i.\n", N);
    printf("Number of processors = %i.\n", procs);
  }

  /* Set number of processors */
  m_set_procs(procs);
}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
  int row, col;
  int len;

  printf("\nInitializing...\n");

  for (col = 0; col < N; col++) {
    for (row = 0; row < N; row++) {
      // printf("col = %d, row = %d, col+N*row = %d\n",col, row, col+N*row);
      A[col+N*row] = (float)rand() / 32768.0;
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
  printf("%5.2f%s", A[col+N*row], (col < N-1) ? ", " : ";\n\t");
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

  double startTime, endTime;
  int myrank, numnodes;

  int norm, row, col;  /* Normalization row, and zeroing  element row and col */
  float multiplier; /*multiplier*/
  int *map;
  int i;
  MPI_Request request;
  MPI_Status status;

  A = (float*)malloc(N*N*sizeof(float));
  B = (float*)malloc(N*sizeof(float));
  X = (float*)malloc(N*sizeof(float));

  /* Initialize MPI*/
  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &numnodes);

  //////////////////////////////////////////////

  /* Process program parameters */
  parameters(argc, argv, myrank);

  if(myrank==0)
  {

    /* Initialize A and B */
    initialize_inputs();

    /* Print input matrices */
    print_inputs();
  }

  /* wait until processor 1 finishes */
  MPI_Barrier(MPI_COMM_WORLD);

  /* sending data to all processors */
  if(myrank == 0)
  {
    for(i=0; i<procs; i++)
    {
      MPI_Isend( &A[0], N*N, MPI_FLOAT, i, 0, MPI_COMM_WORLD,&request);
      MPI_Isend( B,N,MPI_FLOAT, i,0, MPI_COMM_WORLD,&request);
    }
  }
  else
  {
    MPI_Recv( &A[0], N*N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
    MPI_Recv( B,N,MPI_FLOAT, 0,0, MPI_COMM_WORLD,&status);
  }

  /* Test that everyone receives it*/
  // printf("%d : %f\n",myrank, A[0][0]);

  /* assign row to processors */
  map = calloc(N,sizeof(int));

  for(i=0; i<N; i++)
  {
    map[i]= i % procs;
  } 

  MPI_Barrier(MPI_COMM_WORLD);

  /* Start Clock */
  if(myrank == 0)
  {
    printf("\nStarting clock.\n");
    // gettimeofday(&etstart, &tzdummy);
    // etstart2 = times(&cputstart);
    startTime = MPI_Wtime();
  }
  MPI_Barrier(MPI_COMM_WORLD);


  ///////////////////////////////////////////

  /* Gaussian elimination */

  int first_row, last_row, num_rows; // side 0
  int num_remained;
  float s;
  int first_r, last_r, num_r; // other processors side

  /* buffer array for scatter gather */
  int *first_rA = (int*)malloc(procs*sizeof(int));
  int *first_rB = (int*)malloc(procs*sizeof(int));
  int *nfirst_rA=(int*)malloc(procs*sizeof(int));
  int *nfirst_rB=(int*)malloc(procs*sizeof(int));

  /* initialize */
  for ( i = 0; i < procs; i++ )
  {
    first_rA[i] = 0;
    nfirst_rA[i] = 0;
    first_rB[i] = 0;
    nfirst_rB[i] = 0;
  }

  for (norm = 0; norm < N - 1; norm++)
  {

    /* exchange data*/
    // MPI_Bcast( &A[N*norm], N, MPI_FLOAT, 0, MPI_COMM_WORLD );
    // MPI_Bcast( &B[norm], 1, MPI_FLOAT, 0, MPI_COMM_WORLD );
    if (myrank == 0) 
    {
      for (i = 1; i < procs; i++) 
      {
        printf("i = %d\n",i);
        /*Send data to other processes*/
        MPI_Isend(&A[N*norm], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &request);
        MPI_Isend(&B[norm], 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &request);
      }
    }

    /* the rest of the rows*/
    num_remained = N-1 - norm;

    /* divide for processors */
    s =((float)num_remained)/procs;

    // get the first and last row to know the limits
    first_row = norm + 1 + ceil( s * myrank);
    last_row = norm + 1 + floor( s * (myrank+1));

    if ( last_row >= N )
      last_row = N-1;

    num_rows = last_row - first_row +1; // total number of rows

    if ( myrank == 0 ) { // from 0 to other processors the divided workload is sent

      for (i=1; i < procs; i++)
      {
        first_r = norm + 1 + ceil( s*i);
        last_r = norm + 1 + floor( s* (i+1) );

        if( last_r >= N )
          last_r = N-1;

        num_r = last_r - first_r +1;
        if(num_r < 0)
          num_r=0;

        if(first_r>=N)
        {
          first_r=N-1;
          num_r=0;
        }

        first_rA[i]=first_r*N;
        nfirst_rA[i]=num_r*N;
        first_rB[i]=first_r;
        nfirst_rB[i]=num_r;
      }
    }

    /* scatters a buffer in parts to all processes */

    MPI_Scatterv(&A[0], nfirst_rA, first_rA, MPI_FLOAT, &A[first_row*N], N*num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);
    MPI_Scatterv(&B[0], nfirst_rB, first_rB, MPI_FLOAT, &B[first_row], num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);

    for (row = first_row; row <= last_row; row++) {
      multiplier = A[N*row + norm] / A[norm + N*norm];

      for (col = norm; col < N; col++) {
        A[col+N*row] -= A[N*norm + col] * multiplier;
      }
      B[row] -= B[norm] * multiplier;
    }

    /* send results to rank 0 */
    if ( myrank != 0 ) 
    { 
      if ( num_rows > 0  && first_row < N) {
        MPI_Isend( &A[first_row * N], N * num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
        MPI_Isend( &B[first_row],num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
      }
    }
    /* Gathers into specified locations from all processes in a group */
    MPI_Gatherv(&A[first_row* N], N*num_rows, MPI_FLOAT, &A[0],nfirst_rA,first_rA, MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Gatherv(&B[first_row], num_rows, MPI_FLOAT, &B[0],nfirst_rB,first_rB, MPI_FLOAT,0,MPI_COMM_WORLD);
  }

  
  /* (Diagonal elements are not normalized to 1.  This is treated in back substitution.) */
  /* Back substitution */
  if(myrank == 0)
  {
    for (row = N - 1; row >= 0; row--) {
      X[row] = B[row];
      for (col = N-1; col > row; col--) {
        X[row] -= A[col+N*row] * X[col];
      }
      X[row] /= A[col+N*row];
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  //////////////////////////////////////////

  /* Stop Clock */
  if(myrank==0)
  {
    endTime = MPI_Wtime();
    
    /* Display output */
    print_X();

    printf("Elapsed time %f\n", endTime-startTime);
    printf("--------------------------------------------\n");
  }
  // free(A);
  // free(B);
  // free(X);
  MPI_Finalize();

  return 0;

}

