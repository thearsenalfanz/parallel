#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <limits.h>
#include <pthread.h>


/* ---------------------------------------- GLOBALS */
pthread_mutex_t	minimum_value_lock;
int minimum_value;
long partial_list_size;

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

/* ---------------------------------------- RW LOCK */
/* functions needed for FIND_MIN_RW */


/* ---------------------------------------- FIND_MIN_RW */
void *find_min_rw(void *list_ptr) {

	/* vars */
	int *partial_list_pointer = NULL;
	int my_min = minimum_value;
	long i = 0;
	pthread_rwlock_t read_write_lock;
	/* ---- */

	partial_list_pointer = (int *) list_ptr;
	for (i = 0; i < partial_list_size; i++)
		if (partial_list_pointer[i] < my_min)
			my_min = partial_list_pointer[i];

	/* initalize */
	pthread_rwlock_init(&read_write_lock,NULL);

	/* lock the mutex associated with minimum_value and update the variable as required */
	pthread_rwlock_rdlock(&read_write_lock);
	if (my_min < minimum_value) {
		pthread_rwlock_unlock(&read_write_lock);
		pthread_rwlock_wrlock(&read_write_lock);
		minimum_value = my_min;
	}
	/* and unlock the mutex */
	pthread_rwlock_unlock(&read_write_lock);
	pthread_exit(0);
}

/* ---------------------------------------- MAIN */
int main(int argc, char **argv)
{
	/* vars */
	int nt = 10;
	int ret = 0;
	int i = 0;
	long l = 0;
	long cur = 0;
	int seed = 10;
	long nelems = 100;
	double start = 0.;
	double end = 0.;
	int *list = NULL;
	pthread_t *tids = NULL;
	void *res = NULL;
	/* ---- */
	printf("Usage : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
	printf("Default number of threads = %d\nDefault number of elements = %d\nDefault seed value = %d\n", nt, nelems, seed);
	/* parse the command line args */
	while ((ret = getopt(argc, argv, "S:T:N:h")) != -1) {
		switch (ret) {
		case 'h':
			printf("Usage : %s%s", argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
			return 0;
			break;
		case 'S':
			seed = atoi(optarg);
			break;
		case 'N':
			nelems = atol(optarg);
			break;
		case 'T':
			nt = atoi(optarg);
			break;
		case '?':
		default:
			printf("Unknown option : %s%s",
			       argv[0], " -S <seed> -N <num_elems> -T <num_threads> -h\n");
			return -1;
			break;
		}
	}

	/* sanity check */
	if (nt < 1) {
		printf("error : not enough threads\n");
		return -1;
	}
	if (nelems <= (long)(nt)) {
		printf("error : not enough elements\n");
		return -1;
	}
	minimum_value = INT_MAX;

	/* init the mutex */
	pthread_mutex_init(&minimum_value_lock, NULL);

	/* init lists, list_ptr, partial_list_size */
	list = malloc(sizeof(int) * nelems);
	if (list == NULL) {
		printf("Error : could not init the list\n");
		return -1;
	}
	tids = malloc(sizeof(pthread_t) * nt);
	if (tids == NULL) {
		printf("Error : could not init the tids\n");
		return -1;
	}
	srand(seed);
	for (l = 0; l < nelems; l++) {
		list[l] = (long)(rand());
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
	printf("Runtime of %d threads = %f seconds\n", nt, (end - start));

	free(list);
	free(tids);
	tids = NULL;
	list = NULL;

	return 0;
}
/* EOF */
