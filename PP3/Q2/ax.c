#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <mpi.h>
#include <math.h>


/* Program Parameters */
#define MAXN 2000  /* Max value of N */
int N;  /* Matrix size */

int p; // number of processes
int myrank; // rank of the process

/* Matrices and vectors */
float *A, *B, *X;
/* A * X = B, solve for X */


void gauss();

/* returns a seed for srand based on the time */
unsigned int time_seed() {
  struct timeval t;
  struct timezone tzdummy;

  gettimeofday(&t, &tzdummy);
  return (unsigned int)(t.tv_usec);
}

/* Set the program parameters from the command-line arguments */
void parameters(int argc, char **argv) {
  int submit = 0;  /* = 1 if submission parameters should be used */
  int seed = 0;  /* Random seed */

  /* Read command-line arguments */
  srand(time_seed());  /* Randomize */
  if (argc != 2) {
    if ( argc == 1 && !strcmp(argv[1], "submit") ) {
      /* Use submission parameters */
      submit = 1;
      N = 4;
      srand(rand());
    }
    else {
      if (argc == 3) {
	seed = atoi(argv[2]);
	srand(seed);
	if ( myrank == 0 )
   printf("Random seed = %i\n", seed);
      }
      else {
  if ( myrank == 0 ){
	 printf("Usage: %s <matrix_dimension> [random seed]\n", argv[0]);
	 printf("       %s submit\n", argv[0]);
 }
	exit(0);
      }
    }
  }
  /* Interpret command-line args */
  if (!submit) {
    N = atoi(argv[1]);
    if (N < 1 || N > MAXN) {
      if ( myrank == 0 )
        printf("N = %i is out of range.\n", N);
      exit(0);
    }
  }

  /* Print parameters */
  if ( myrank == 0 )
    printf("\nMatrix dimension N = %i.\n", N);

}

/* Initialize A and B (and X to 0.0s) */
void initialize_inputs() {
  int row, col;


  for (col = 0; col < N; col++) {
    for (row = 0; row < N; row++) {
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

int main(int argc, char **argv) {

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  MPI_Comm_size(MPI_COMM_WORLD, &p);

  double startTime, endTime;

if(myrank==0)
  printf("--------------------------------------------\n");
  //printf("Parallel Gaussian Elimination\n\n");

  /* Process program parameters */
  parameters(argc, argv);

// alocate memory for A, B and X
  A = (float*)malloc(N*N*sizeof(float));
  B = (float*)malloc(N*sizeof(float));
  X = (float*)malloc(N*sizeof(float));

if(myrank==0){
  /* Initialize A and B */
  initialize_inputs();

  /* Print input matrices */
  print_inputs();
}

MPI_Barrier(MPI_COMM_WORLD); // sync the process which increases performance
 if (myrank == 0){
   startTime=MPI_Wtime();
 }
  /* Gaussian Elimination */
  gauss();

 endTime=MPI_Wtime();


  /* Display output */
if(myrank==0)  {
  print_X();


  /* Display timing results */
  printf("\nElapsed time = %g ms.\n", (float)(endTime -  startTime)); ///(float)1000);

  printf("--------------------------------------------\n");
}

// so that each process finishes at the same time
MPI_Barrier(MPI_COMM_WORLD);
free(A);
free(B);
free(X);
MPI_Finalize();
return 0;
}

 void gauss() {
   int norm, row, col;  /* Normalization row, and zeroing element row and col */
   float multiplier;
   int i;
   int first_row, last_row, num_rows; // side 0
   int remaining_rows;
   float s;
   int first_r, last_r, num_r; // other processors side

   MPI_Request request;

   //buffer array for scatter gather
   int *first_rA=(int*)malloc(p*sizeof(int));
   int *first_rB=(int*)malloc(p*sizeof(int));
   int *nfirst_rA=(int*)malloc(p*sizeof(int));
   int *nfirst_rB=(int*)malloc(p*sizeof(int));


   for ( i = 0; i < p; i++ )
    {
         first_rA[i] = 0;
         nfirst_rA[i] = 0;
         first_rB[i] = 0;
         nfirst_rB[i] = 0;
     }

   for (norm = 0; norm < N - 1; norm++) {

    // each process needs these values
     MPI_Bcast( &A[ N*norm], N, MPI_FLOAT, 0, MPI_COMM_WORLD );
     MPI_Bcast( &B[norm], 1, MPI_FLOAT, 0, MPI_COMM_WORLD );

      // we need to know how many rows will processors handle to diviede the work
     remaining_rows = N-1 - norm;
     s =((float)remaining_rows)/p; // divide it to number of processors

     // get the first and last row to know the limits
      first_row = norm + 1 + ceil( s*myrank);
      last_row = norm + 1 + floor( s*(myrank+1));

     if ( last_row >= N )
        last_row = N-1;

      num_rows = last_row - first_row +1; // total number of rows

     if ( myrank == 0 ) { // from 0 to other processors the divided workload is sent

         for (i=1; i < p; i++) {
              first_r = norm + 1 + ceil( s*i);
              last_r = norm + 1 + floor( s* (i+1) );

             if( last_r >= N )
                  last_r = N-1;
          //    if(first_r <1)
          //        first_r=0;

              num_r = last_r - first_r +1;
              if(num_r<0)
                  num_r=0;

              if(first_r>=N){
                    first_r=N-1;
                    num_r=0;
                  }

            first_rA[i]=first_r*N;
            nfirst_rA[i]=num_r*N;
            first_rB[i]=first_r;
            nfirst_rB[i]=num_r;


         }
     }

     MPI_Scatterv(&A[0], nfirst_rA, first_rA, MPI_FLOAT, &A[first_row*N], N*num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);
     MPI_Scatterv(&B[0], nfirst_rB, first_rB, MPI_FLOAT, &B[first_row], num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);


// gaussian elimination
                 for (row = first_row; row <= last_row; row++) {
                     multiplier = A[N*row + norm] / A[norm + N*norm];
                     for (col = norm; col < N; col++) {
                         A[col+N*row] -= A[N*norm + col] * multiplier;
                     }
                     B[row] -= B[norm] * multiplier;
                 }


             if ( myrank != 0 ) { // from other processors to 0 so that 0 can receive the results
                 if ( num_rows > 0  && first_row < N) {
                     MPI_Isend( &A[first_row * N], N * num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
                     MPI_Isend( &B[first_row],num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
                 }
             }

             MPI_Gatherv(&A[first_row* N], N*num_rows, MPI_FLOAT, &A[0],nfirst_rA,first_rA, MPI_FLOAT,0,MPI_COMM_WORLD);
             MPI_Gatherv(&B[first_row], num_rows, MPI_FLOAT, &B[0],nfirst_rB,first_rB, MPI_FLOAT,0,MPI_COMM_WORLD);

         }

   /* (Diagonal elements are not normalized to 1.
    * This is treated in back
    * substitution.)
    */
      if(myrank==0){
     /* Back substitution */
     for (row = N - 1; row >= 0; row--) {
       X[row] = B[row];
       for (col = N-1; col > row; col--) {
         X[row] -= A[N*row+col] * X[col];
       }
       X[row] /= A[N*row+row];
     }
 }
 }
