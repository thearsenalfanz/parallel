#include <stdio.h>
#include <stdlib.h>

void *eliminate(void *param)
{
    int i = *((int *) param);
    printf("Print i = %d\n", i);
}

int main(){
	int t,i;
	int procs = 2;
	int *index = calloc(procs, sizeof(int));

	for (i = 0; i < procs; i++)
	{
		index[i] = i;
		printf("index[%d]\n",i );
	}

	for (i=0; i < procs*2; i++)
	{

		for (t = 0; t < procs; t++) {
    		/* create threads */
			printf("INDEX %d = %d\n",t,index[t] );
			eliminate(&index[t]);
		}
	}


}