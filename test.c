#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void *eliminate(void *param)
{
    int i = *((int *) param);
    printf("Print i = %d\n", i);
}

void test(int N,int procs)
{
	int i=0;
	int p = 1+((N-1)/procs); /* roundup(N/procs) */
	int x = N-1;
	int index[procs][p];
	int counter[procs];
	int k,j;
	int z;

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
	for (i = 0; i < x; i++)
	{
		z = i % procs;
		printf("%d mod %d = %d\n",i,procs, i % procs);
		printf("i=%d , x= %d\n",i,x);
		if( i != x)
		{
			printf("%d %d\n",i,x);
			index[z][counter[z]] = i;
			printf("z=%d, count =%d,value=%d\n",z,counter[z],index[z][counter[z]]);
			index[z][counter[z]+1] = x;
			printf("z=%d, count =%d,value=%d\n",z,counter[z]+1,index[z][counter[z]+1]);
			counter[z] = counter[z]+2;
		}
		else
		{
			printf("%d\n",i);
			index[z][counter[z]] = i;
			printf("z=%d, count =%d,value=%d\n",z,counter[z],index[z][counter[z]]);
			counter[z]++;
		}
		x--;
	}

	printf("\n\n***procs =%d\n",procs );

	for ( i = 0; i < procs; i++ ) {
		printf("[thread: %d] ",i);
		for ( j = 0; j < p; j++ ) {
			printf("%d ",index[i][j]);
		}
		printf("\n");
	}
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
	test(10,2);


}