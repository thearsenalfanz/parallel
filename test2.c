#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

void *eliminate(void *param)
{
    int i = *((int *) param);
    printf("Print i = %d\n", i);
}

void test(int N,int procs)
{
	int i=0;
	int p = ceil(N/procs);
	int x = N-1;
	int index[procs][p-1];
	int counter[procs];
	int k,j;
	int tid;

	printf("**********ceil = %d\n",ceil(2.5));
	printf("**********p = %d\n",p);

	for ( i = 0; i < procs; i++ ) {
		counter[i] = 0;
		for(j=0; j<p; j++)
			index[i][j] = -1;
	}

	for ( i = 0; i < procs; i++ ) {
		printf("[thread: %d] ",i);
		for ( j = 0; j < p; j++ ) {
			printf("%d ",index[i][j]);
		}
		printf("\n");
	}
	for (i = 0; i <= p; i++)
	{
		tid = i % procs;
		printf("%d mod %d = %d\n",i,procs, i % procs);
		if(i != x)
		{
			printf("%d %d\n",i,x);
			index[tid][counter[tid]] = i;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid],index[tid][counter[tid]]);
			index[tid][counter[tid]+1] = x;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid]+1,index[tid][counter[tid]+1]);
			counter[tid] = counter[tid]+2;
		}
		else
		{
			printf("%d\n",i);
			index[tid][counter[tid]] = i;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid],index[tid][counter[tid]]);
			counter[tid]++;
		}
		x--;
	}

	for ( i = 0; i < procs; i++ ) {
		printf("[thread: %d] ",i);
		for ( j = 0; j < p; j++ ) {
			printf("%d ",index[i][j]);
		}
		printf("\n");
	}
}

int **indexing(int N,int procs)
{
	int i,j;
	int p = N/procs;
	int x = N-1;
	int **index;
	int counter[procs];
	int tid; /* mod value = tid*/

	/* initialize array */
	index = malloc( procs * sizeof(int *));
	for ( i = 0; i < procs; i++ ) {
		counter[i] = 0;
		index[i] = malloc( (p+1) * sizeof(int *));
		for(j=0; j<p; j++)
			index[i][j] = -1;
	}

	for ( i = 0; i < procs; i++ ) {
		printf("[thread: %d] ",i);
		for ( j = 0; j < p; j++ ) {
			printf("%d ",index[i][j]);
		}
		printf("\n");
	}
	/* assign index to threads */
	for (i = 0; i <= p; i++)
	{
		tid = i % procs;
		if(i != x)
		{
			index[tid][counter[tid]] = i;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid],index[tid][counter[tid]]);
			index[tid][counter[tid]+1] = x;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid]+1,index[tid][counter[tid]+1]);
			counter[tid] = counter[tid]+2;
		}
		else
		{
			index[tid][counter[tid]] = i;
			printf("tid=%d, count =%d,value=%d\n",tid,counter[tid],index[tid][counter[tid]]);
			counter[tid]++;
		}
		x--;
	}
	return index;
}

int main(){
	// int t,i;
	// int procs = 2;
	// int *index = calloc(procs, sizeof(int));

	// for (i = 0; i < procs; i++)
	// {
	// 	index[i] = i;
	// 	printf("index[%d]\n",i );
	// }

	// for (i=0; i < procs*2; i++)
	// {

	// 	for (t = 0; t < procs; t++) {
 //    		/* create threads */
	// 		printf("INDEX %d = %d\n",t,index[t] );
	// 		eliminate(&index[t]);
	// 	}
	// }
	int **index;
	int i,j;
	int procs = 3;
	int N = 10;
	test(N,procs);
	printf("===================================\n");
	index = indexing(N,procs);
	for ( i = 0; i < LEN(index); i++ ) {
		printf("[thread: %d] ",i);
		for ( j = 0; j < LEN(index[0]); j++ ) {
			printf("%d ",index[i][j]);
		}
		printf("\n");
	}

}