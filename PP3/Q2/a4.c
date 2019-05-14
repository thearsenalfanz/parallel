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


#define MAXN 8000 /* Max value of N */
int N; /* Matrix size */
#define DIVFACTOR 32768.0f

#define SOURCE 0

/* My process rank           */
int myrank;
/* The number of processes   */
int procs; 

/* Matrixes given by a pointer */
float *A, *B, *X;



/* returns a seed for srand based on the time */
unsigned int time_seed() {
  struct timeval t;
  struct timezone tzdummy;

  gettimeofday(&t, &tzdummy);
  return (unsigned int)(t.tv_usec);
}


/* Set the program parameters from the command-line arguments */
void parameters(int argc, char **argv, int myrank) {
  int submit = 0;  /* = 1 if submission parameters should be used */
  int seed = 0;  /* Random seed */

  /* Read command-line arguments */
  srand(time_seed());  /* Randomize */
  if (argc != 3) {
    if ( argc == 2 && !strcmp(argv[1], "submit") ) {
      /* Use submission parameters */
      submit = 1;
      N = 4;
      procs = 2;
      srand(rand());
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
    // if (procs > m_get_numprocs()) {
    //   printf("Warning: %i processors requested; only %i available.\n",
    //    procs, m_get_numprocs());
    //   procs = m_get_numprocs();
    // }
  }

  /* Print parameters */
  if(myrank == 0)
  {
    printf("Random seed = %i\n", seed);
    printf("\nMatrix dimension N = %i.\n", N);
    printf("Number of processors = %i.\n", procs);
  }

  /* Set number of processors */
  // m_set_procs(procs);
}



/* Allocates memory for A, B and X */
void allocate_memory() {
    A = (float*)malloc( N*N*sizeof(float) );
    B = (float*)malloc( N*sizeof(float) );
    X = (float*)malloc( N*sizeof(float) );
}

/* Free allocated memory for arrays */
void free_memory() {
    free(A);
    free(B);
    free(X);
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

/* Print matrix A */
void print_A() {
  int row, col;

  if (N < 10) {
    printf("\nA =\n\t");
    for (row = 0; row < N; row++) {
      for (col = 0; col < N; col++) {
    printf("%5.2f%s", A[col+N*row], (col < N-1) ? ", " : ";\n\t");
      }
    }
  }
}
/* Print matrix B */
void print_B() {
  int col;
  if (N < 10) {
      printf("\nB = [");
        for (col = 0; col < N; col++) {
          printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
        }
    }
}
/* Print matrix X */
void print_X() {
  int row;

  if (N < 100) {
    printf("\nX = [");
    for (row = 0; row < N; row++) {
      printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
    }
  }
}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {

  int row, col;

  printf("\nInitializing...\n");
  for (row = 0; row < N; row++) {
    for (col = 0; col < N; col++) {
      A[col+N*row] = (float)rand() / DIVFACTOR;
    }
    B[row] = (float)rand() / DIVFACTOR;
    X[row] = 0.0;
  }

}


int main(int argc, char **argv) {

    /* Prototype functions*/
    void gauss();

    MPI_Init(&argc, &argv);

    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    /* Every process reads the parameters to prepare dimension */
    parameters(argc, argv, myrank);

    /* Every process must allocate memory for the arrays */
    allocate_memory();

    if ( myrank == SOURCE ) {
        /* Initialize A and B */
        initialize_inputs();

        /* Print input matrices */
        print_inputs();
    }

    /*printf("\nProcess number %d of %d says hi\n",
            myrank+1, p);*/

    gauss();

    if ( myrank == 0 ) {

        /* Print input matrices */
        print_A();
        print_B();
        print_X();
    }

    /* The barrier prevents any process to reach the finalize before the others have finished their communications */
    MPI_Barrier(MPI_COMM_WORLD);

    /* Free memory used for the arrays that we allocated previously */
    free_memory();

    MPI_Finalize();
}


/* Includes both algorithms */
void gauss() {

    void gaussElimination();
    void backSubstitution();

    /* Times */
    double t1, t2;

    /* Barrier to sync all processes before starting the algorithms */
    MPI_Barrier(MPI_COMM_WORLD);

    /* Initial time */
    if ( myrank == SOURCE )
        t1 = MPI_Wtime();

    /* Gauss Elimination is performed using MPI */
    gaussElimination();

    /* Back Substitution is performed sequentially */
    if ( myrank == SOURCE ) {
        backSubstitution();

        /* Finish time */
        t2 = MPI_Wtime();

        printf("\nElapsed time: %f miliseconds\n", (t2-t1) * 1000 );
    }
    
}



/* Guassian Elimination algorithm using MPI */
void gaussElimination() {

    MPI_Status status;
    MPI_Request request;
    int row, col, i, norm;
    float multiplier;

  /* Gaussian elimination */

  int *A_first = (int*) malloc ( procs * sizeof(int) );
  int *num_rows_A = (int*) malloc ( procs * sizeof(int) );
  int *B_first = (int*) malloc ( procs * sizeof(int) );
  int *num_rows_B = (int*) malloc ( procs * sizeof(int) );
  for ( i = 0; i < procs; i++ ) {
    A_first[i] = 0;
    num_rows_A[i] = 0;
    B_first[i] = 0;
    num_rows_B[i] = 0;
  }

  /* Gaussian elimination */
  /* Outer loop. A new column will have all 0s down the [norm] */
  for (norm = 0; norm < N-1; norm++) {

    /* Broadcast values A[norm] and B[norm] */
    // MPI_Bcast( &A[N*norm], N, MPI_FLOAT, 0, MPI_COMM_WORLD );
    // MPI_Bcast( &B[norm], 1, MPI_FLOAT, 0, MPI_COMM_WORLD );
    if(myrank == 0)
    {
      for(i=0; i<procs; i++)
      {
        MPI_Isend( &A[N*norm], N, MPI_FLOAT, i, 0, MPI_COMM_WORLD,&request);
        MPI_Isend( &B[norm], 1 , MPI_FLOAT, i,0, MPI_COMM_WORLD,&request);
      }
    }
    else
    {
      MPI_Recv( &A[N*norm], N, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
      MPI_Recv( &B[norm], 1 ,MPI_FLOAT, 0,0, MPI_COMM_WORLD,&status);
    }

    /* Test that everyone receives it*/
    // printf("%d : %f\n",myrank, A[0]);


    /* number of rows to be calculated for each process */
    int subset = N - 1 - norm;
    float step = ((float)subset ) / (procs);
    
    /* Sets of rows */
    int first_row = norm + 1 + ceil( step * (myrank) );
    int last_row = norm + 1 + floor( step * (myrank+1) );
    if ( last_row >= N ) 
      last_row = N-1;
    int t_num_rows = last_row - first_row +1;


    /* send data from rank 0 to other processes */

    if ( myrank == 0 ) {

      for ( i = 1; i < procs; i++ ) {

        /* Assign data to each process */
        int first = norm + 1 + ceil( step * (i) );
        int last = norm + 1 + floor( step * (i+1) );
        if( last >= N ) 
          last = N -1;
        int num_rows = last - first +1;

        if ( num_rows < 0 ) 
          num_rows = 0;
        if ( first >= N )
        {
          num_rows = 0; 
          first = N-1; 
        }

        A_first[i] = first * N;
        B_first[i] = first;
        num_rows_A[i] = num_rows * N;
        num_rows_B[i] = num_rows;
        MPI_Isend( &A[first * N], N * num_rows, MPI_FLOAT, i,0, MPI_COMM_WORLD, &request);
        MPI_Isend( &B[first], num_rows, MPI_FLOAT, i,0, MPI_COMM_WORLD, &request);
      }

    }
    /* Receive */
    else {
      if ( t_num_rows > 0  && first_row < N) 
      {
        MPI_Recv( &A[first_row * N], N * t_num_rows, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv( &B[first_row], t_num_rows, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, &status);
        }
    }

    if ( t_num_rows > 0  && first_row < N) {  
      for (row = first_row; row <= last_row; row++) {

        multiplier = A[N*row + norm] / A[norm + N*norm];
        for (col = norm; col < N; col++) {
          A[col+N*row] -= A[N*norm + col] * multiplier;
        }

        B[row] -= B[norm] * multiplier;
      }
    }

    /* send results*/

    if ( myrank != 0 ) {
      if ( t_num_rows > 0  && first_row < N) {
        MPI_Isend( &A[first_row * N], N * t_num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD, &request);
        MPI_Isend( &B[first_row], t_num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD, &request);
      }
    }
    else {
      for ( i = 1; i < procs; i++ ) {
        if( num_rows_B[i] < 1  || B_first[i] >= N) 
          continue;
        MPI_Recv( &A[ A_first[i] ], num_rows_A[i] , MPI_FLOAT, i,0, MPI_COMM_WORLD, &status );
        MPI_Recv( &B[ B_first[i] ], num_rows_B[i] , MPI_FLOAT, i,0, MPI_COMM_WORLD, &status );
      }
    }
  }
}


/* Back substitution sequential algorithm */
void backSubstitution () {

    int norm, row, col;  /* Normalization row, and zeroing
      * element row and col */

    /* Back substitution */
    for (row = N - 1; row >= 0; row--) {
        X[row] = B[row];

        for (col = N-1; col > row; col--) {
            X[row] -= A[N*row+col] * X[col];
        }
        
        X[row] /= A[N*row + row];
    }
}