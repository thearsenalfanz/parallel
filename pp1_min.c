#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>
#include "gnuplot_i.h"

/* ---------------------------------------- GLOBALS */
pthread_mutex_t	minimum_value_lock;
int minimum_value;
long partial_list_size;
int err;
int *list = NULL;
double duration[4];
double duration_rw[4];

/* ---------------------------------------- STRUCT  */
typedef struct {
	int readers;
	int writer;
	pthread_cond_t readers_proceed;
	pthread_cond_t writer_proceed;
	int pending_writers;
	pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

/* ---------------------------------------- MYSECOND */
static double 
mysecond()
{
	struct timeval	tp;
	struct timezone	tzp;
	int i = 0;

	i = gettimeofday(&tp, &tzp);
	return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

/* ---------------------------------------- FIND_MIN */
void *find_min(void *list_ptr)
{
	/* vars */
	int *partial_list_pointer = NULL;
	int my_min = 0;
	long i = 0;
	/* ---- */

	partial_list_pointer = (int *)(list_ptr);

	my_min = partial_list_pointer[0];
	for (i = 0; i < partial_list_size; i++) {
		if (partial_list_pointer[i] < my_min) {
			my_min = partial_list_pointer[i];
		}
	}

	/* lock and update the global copy */
	pthread_mutex_lock(&minimum_value_lock);

	if (my_min < minimum_value) {
		minimum_value = my_min;
	}
	pthread_mutex_unlock(&minimum_value_lock);

	pthread_exit(0);
}

/* ---------------------------------------- INIT RW LOCK */

void mylib_rwlock_init (mylib_rwlock_t *l) {
	l -> readers = l -> writer = l -> pending_writers = 0;
	pthread_mutex_init(&(l -> read_write_lock), NULL);
	pthread_cond_init(&(l -> readers_proceed), NULL);
	pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
	/* if there is a write lock or pending writers, perform condition wait.. else increment count of readers and grant read lock */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> pending_writers > 0) || (l -> writer > 0))
		pthread_cond_wait(&(l -> readers_proceed), &(l -> read_write_lock));
	l -> readers ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
	/* if there are readers or writers, increment pending writers count and wait. On being woken, decrement pending writers count and increment writer count */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> writer > 0) || (l -> readers > 0)) {
		l -> pending_writers ++;
		pthread_cond_wait(&(l -> writer_proceed),
			&(l -> read_write_lock));
	}
	l -> pending_writers --;
	l -> writer ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
	/* if there is a write lock then unlock, else if there are read locks, 
	decrement count of read locks. If the count is 0 and there is a pending writer, 
	let it through, else if there are pending readers, let them all go through */
	pthread_mutex_lock(&(l -> read_write_lock));
	if (l -> writer > 0)
		l -> writer = 0;
	else if (l -> readers > 0)
		l -> readers --;
	pthread_mutex_unlock(&(l -> read_write_lock));
	if ((l -> readers == 0) && (l -> pending_writers > 0))
		pthread_cond_signal(&(l -> writer_proceed));
	else if (l -> readers > 0)
		pthread_cond_broadcast(&(l -> readers_proceed));
}


/* ---------------------------------------- FIND_MIN_RW */
void *find_min_rw(void *list_ptr) {

	/* vars */
	int *partial_list_pointer = NULL;
	int my_min = 0;
	long i = 0;
	mylib_rwlock_t read_write_lock;
	/* ---- */

	partial_list_pointer = (int *) list_ptr;
	for (i = 0; i < partial_list_size; i++)
		if (partial_list_pointer[i] < my_min)
			my_min = partial_list_pointer[i];

	/* initalize */
	mylib_rwlock_init (&read_write_lock);

	/* lock the mutex associated with minimum_value and update the variable as required */
	mylib_rwlock_rlock(&read_write_lock);
	if (my_min < minimum_value) {
		mylib_rwlock_unlock(&read_write_lock);
		mylib_rwlock_wlock(&read_write_lock);
		minimum_value = my_min;
	}
	/* and unlock the mutex */
	mylib_rwlock_unlock(&read_write_lock);
	pthread_exit(0);
}

/* ---------------------------------------- INIT  */
/* Using read-write locks for computing the minimum of a list of numbers */
int init(int nt,int nelems, int index)
{
	/* vars */
	int i = 0;
	long cur = 0;
	double start = 0.;
	double end = 0.;
	pthread_t *tids = NULL;
	void *res = NULL;
	/* ---- */

	minimum_value = INT_MAX;

	/* init the mutex */
	pthread_mutex_init(&minimum_value_lock, NULL);
	

	/* sanity check */
	if (nt < 1) {
		printf("error : not enough threads\n");
		return -1;
	}
	if (nelems <= (long)(nt)) {
		printf("error : not enough elements\n");
		return -1;
	}

	tids = malloc(sizeof(pthread_t) * nt);
	if (tids == NULL) {
		printf("Error : could not init the tids\n");
		return -1;
	}

	if (nt == 1) {
		partial_list_size = nelems;
	} else {
		partial_list_size = (nelems / (long)(nt)) + (nelems % (long)(nt));
	}


	start = mysecond();
	/* create threads */
	for (i = 0; i < nt; i++) {
		if (pthread_create(&tids[i], NULL, &find_min, &list[cur]) != 0) {
			printf("Error : pthread_create failed on spawning thread %d\n", i);
			return -1;
		}
		cur += partial_list_size;

		/*
		 * we do this check in order to ensure that our threads
		 * down't go out of bounds of the list
		 * 
		 */
		if ((cur + partial_list_size) > nelems) {
			cur = nelems - partial_list_size;
		}
	}

	/* join threads */
	for (i = 0; i < nt; i++) {
		if (pthread_join(tids[i], &res) != 0) {
			printf("Error : pthread_join failed on joining thread %d\n", i);
			return -1;
		}
	}

	end = mysecond();

	printf("Minimum value found: %d\n", minimum_value);
	duration[index] = (end - start);
	printf("Runtime of %d threads = %f seconds\n", nt, duration[index]);
	free(tids);
	tids = NULL;

	return 0;

}

/* ---------------------------------------- INIT_RW */
/* Using read-write locks for computing the minimum of a list of numbers */
double init_rw(int nt,int nelems,int index)
{
	/* vars */
	int i = 0;
	long cur = 0;
	double start = 0.;
	double end = 0.;
	double duration = 0.0;
	pthread_t *tids = NULL;
	void *res = NULL;
	/* ---- */

	minimum_value = INT_MAX;

	/* init the mutex */
	pthread_mutex_init(&minimum_value_lock, NULL);
	

	/* sanity check */
	if (nt < 1) {
		printf("error : not enough threads\n");
		return -1;
	}
	if (nelems <= (long)(nt)) {
		printf("error : not enough elements\n");
		return -1;
	}

	tids = malloc(sizeof(pthread_t) * nt);
	if (tids == NULL) {
		printf("Error : could not init the tids\n");
		return -1;
	}

	if (nt == 1) {
		partial_list_size = nelems;
	} else {
		partial_list_size = (nelems / (long)(nt)) + (nelems % (long)(nt));
	}


	start = mysecond();
	/* create threads */
	for (i = 0; i < nt; i++) {
		if (pthread_create(&tids[i], NULL, &find_min_rw, &list[cur]) != 0) {
			printf("Error : pthread_create failed on spawning thread %d\n", i);
			return -1;
		}
		cur += partial_list_size;

		/*
		 * we do this check in order to ensure that our threads
		 * down't go out of bounds of the list
		 * 
		 */
		if ((cur + partial_list_size) > nelems) {
			cur = nelems - partial_list_size;
		}
	}

	/* join threads */
	for (i = 0; i < nt; i++) {
		if (pthread_join(tids[i], &res) != 0) {
			printf("Error : pthread_join failed on joining thread %d\n", i);
			return -1;
		}
	}

	end = mysecond();

	printf("Minimum value found: %d\n", minimum_value);
	duration_rw[index] = (end - start);
	printf("Runtime of %d threads = %f seconds\n", nt, duration_rw[index]);

	free(tids);
	tids = NULL;

	return 0;

}

/* ---------------------------------------- MAIN */
int main()
{
	/* vars */
	int i = 0;
	long l = 0;
	long nelems = 100000000;
	int seed = 10;
	gnuplot_ctrl *h;
	/* ---- */

	printf("Number of elements = %d\nSeed value = %d\n\n", nelems, seed);

	/* init lists, list_ptr, partial_list_size */
	list = malloc(sizeof(int) * nelems);
	if (list == NULL) {
		printf("Error : could not init the list\n");
		return -1;
	}

	/* randomly generate numbers */
	
	srand(seed);
	for (l = 0; l < nelems; l++) {
		list[l] = (long)(rand());
	}

	/* see results of from using thread 1,2,4 and 8 using find_min */
	printf("----------RESULTS FROM FIND_MIN----------\n");
	err = init(1,nelems,0);
	if(err == -1)
		return -1;
	err = init(2,nelems,1);
	if(err == -1)
		return -1;
	err = init(4,nelems,2);
	if(err == -1)
		return -1;
	err = init(8,nelems,3);
	if(err == -1)
		return -1;
	printf("------------------------------------------\n");

	/* see results of from using thread 1,2,4 and 8 using find_min_rw */
	printf("----------RESULTS FROM FIND_MIN_RW----------\n");
	err = init_rw(1,nelems,0);
	if(err == -1)
		return -1;
	err = init_rw(2,nelems,1);
	if(err == -1)
		return -1;
	err = init_rw(4,nelems,2);
	if(err == -1)
		return -1;
	err = init_rw(8,nelems,3);
	if(err == -1)
		return -1;
	printf("------------------------------------------\n");

	free(list);
	list = NULL;

	/* plot performance */
    h = gnuplot_init();
    gnuplot_plot_x(h, duration, 4, "FIND MIN") ;
    sleep(2);
    gnuplot_close(h);

	return 0;

}


/* EOF */
