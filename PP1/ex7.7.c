typedef struct {
	int readers;
	int writer;
	pthread_cond_t readers_proceed;
	pthread_cond_t writer_proceed;
	int pending_writers;
	pthread_mutex_t read_write_lock;
} mylib_rwlock_t;

void mylib_rwlock_init (mylib_rwlock_t *l) {
	l -> readers = l -> writer = l -> pending_writers = 0;
	pthread_mutex_init(&(l -> read_write_lock), NULL);
	pthread_cond_init(&(l -> readers_proceed), NULL);
	pthread_cond_init(&(l -> writer_proceed), NULL);
}

void mylib_rwlock_rlock(mylib_rwlock_t *l) {
	/* if there is a write lock or pending writers, perform condition
	wait.. else increment count of readers and grant read lock */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> pending_writers > 0) || (l -> writer > 0))
		pthread_cond_wait(&(l -> readers_proceed),
			&(l -> read_write_lock));
	l -> readers ++;
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_wlock(mylib_rwlock_t *l) {
	/* if there are readers or writers, increment pending writers
	count and wait. On being woken, decrement pending writers
	count and increment writer count */
	pthread_mutex_lock(&(l -> read_write_lock));
	while ((l -> writer > 0) || (l -> readers > 0)) {
		l -> pending_writers ++;
		pthread_cond_wait(&(l -> writer_proceed),
			&(l -> read_write_lock));
	}
	l -> pending_writers --;
	l -> writer ++
	pthread_mutex_unlock(&(l -> read_write_lock));
}

void mylib_rwlock_unlock(mylib_rwlock_t *l) {
	/* if there is a write lock then unlock, else if there are
	read locks, decrement count of read locks. If the count
	is 0 and there is a pending writer, let it through, else
	if there are pending readers, let them all go through */
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

void *find_min_rw(void *list_ptr) {

	/* vars */
	int *partial_list_pointer = NULL;
	int my_min = 0;
	long i = 0;
	/* ---- */

	partial_list_pointer = (int *) list_ptr;
	for (i = 0; i < partial_list_size; i++)
		if (partial_list_pointer[i] < my_min)
			my_min = partial_list_pointer[i];
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