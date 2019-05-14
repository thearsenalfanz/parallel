#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

void print_inputs(float *A, float *B, int N) {
  int row, col;
  int i = 0;

  if (N < 10)
  {
    printf("\nA =\n\t");
    for (row = 0; row < N; row++)
    {
      for (col = 0; col < N; col++)
      {
        A[col+N*row] = i;
        printf("%5.2f%s", A[col+N*row], (col < N-1) ? ", " : ";\n\t");
        i++;
      }
    }
    i=0;
    printf("\nB = [");
    for (col = 0; col < N; col++) {
      B[col] = i;
      printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
      i++;
    }
  }
}

int main(int argc, char **argv) {

  int N = 4;
  int myrank = 1;
  int procs = 4;

  float *A, *B, *X;

  int norm, row, col;  /* Normalization row, and zeroing  element row and col */
  float multiplier; /*multiplier*/

  int first_row, last_row, num_rows; // side 0
  int num_remained;
  float s;
  int first_r, last_r, num_r; // other processors side

  int i;

  // MPI_Request request;

  A = (float*)malloc(N*N*sizeof(float));
  B = (float*)malloc(N*sizeof(float));
  X = (float*)malloc(N*sizeof(float));

  print_inputs(A,B,N);


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
    printf("\n\nnorm = %d\n", norm);

    /* exchange data*/
    printf("broadcast A %f until N\n", A[N*norm]);
    printf("broadcast B %f until N\n", B[norm]);
    // MPI_Bcast( &A[N*norm], N, MPI_FLOAT, 0, MPI_COMM_WORLD );
    // MPI_Bcast( &B[norm], 1, MPI_FLOAT, 0, MPI_COMM_WORLD );

    /* the rest of the rows*/
    num_remained = N-1 - norm;
    printf("num_remained = %d\n", num_remained);

    /* divide for processors */
    s =((float)num_remained)/procs;

    // get the first and last row to know the limits
    first_row = norm + 1 + ceil( s * myrank);
    printf("first_row = %d\n", first_row);
    last_row = norm + 1 + floor( s * (myrank+1));
    printf("last_row = %d\n", last_row);

    if ( last_row >= N )
      last_row = N-1;

    num_rows = last_row - first_row +1; // total number of rows

    if ( myrank == 0 ) { // from 0 to other processors the divided workload is sent

      for (i=1; i < procs; i++)
      {
        first_r = norm + 1 + ceil( s*i);
        last_r = norm + 1 + floor( s* (i+1) );
        printf("first_r = %d\n", first_r);
        printf("last_r = %d\n", last_r);
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

        printf("first_rA[%d] = %d\n",i, first_r*N);
        printf("nfirst_rA[%d] = %d\n",i, num_r*N);
        printf("first_rB[%d] = %d\n",i, first_r);
        printf("nfirst_rB[%d] = %d\n",i, num_r);
      }
    }

    /* scatters a buffer in parts to all processes */
    printf("scatter A %f , number to send = %d, displacement = %f \n",A[0],nfirst_rA[0],first_rA[0] );
    printf("receive A at %d for %d \n", A[first_row*N], (N*num_rows) );
    printf("scatter B %f , number to send = %d, displacement = %f \n",A[0],nfirst_rA[0],first_rA[0] );
    printf("receive B at %d for %d \n", B[first_row], num_rows );
    // MPI_Scatterv(&A[0], nfirst_rA, first_rA, MPI_FLOAT, &A[first_row*N], N*num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);
    // MPI_Scatterv(&B[0], nfirst_rB, first_rB, MPI_FLOAT, &B[first_row], num_rows, MPI_FLOAT,0, MPI_COMM_WORLD);

    // for (row = first_row; row <= last_row; row++) {
    //   multiplier = A[N*row + norm] / A[norm + N*norm];

    //   for (col = norm; col < N; col++) {
    //     A[col+N*row] -= A[N*norm + col] * multiplier;
    //   }
    //   B[row] -= B[norm] * multiplier;
    // }
    printf("..calculate..\n");

    /* send results to rank 0 */
    if ( myrank != 0 ) 
    { 
      if ( num_rows > 0  && first_row < N) {
        printf("sending A %f until %d\n", A[first_row * N], N * num_rows);
        printf("sending B %f until %d\n", B[first_row], N * num_rows);
        // MPI_Isend( &A[first_row * N], N * num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
        // MPI_Isend( &B[first_row],num_rows, MPI_FLOAT, 0,0, MPI_COMM_WORLD,&request);
      }
    }
    /* Gathers into specified locations from all processes in a group */
    printf("gather A %f until %d\n", A[first_row * N], N * num_rows);
    printf("gather B %f until %d\n", B[first_row], N * num_rows);
    // MPI_Gatherv(&A[first_row* N], N*num_rows, MPI_FLOAT, &A[0],nfirst_rA,first_rA, MPI_FLOAT,0,MPI_COMM_WORLD);
    // MPI_Gatherv(&B[first_row], num_rows, MPI_FLOAT, &B[0],nfirst_rB,first_rB, MPI_FLOAT,0,MPI_COMM_WORLD);

  }
  return 0;
}
