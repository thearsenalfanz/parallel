void *find_min_rw(void *list_ptr) {

	/* vars */
	int *partial_list_pointer = NULL;
	int my_min = 0;
	long i = 0;
	/* ---- */

	
	int *partial_list_pointer, my_min, i;
	my_min = MIN_INT;
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