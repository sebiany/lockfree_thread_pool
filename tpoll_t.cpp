#include "tpool_t.h"
#include "cst_exp.h"


tpool_t::tpool_t(int num): num_threads(num) {
	try {
		if (num_threads <= 0)
			throw cst_exp();
	} catch (cst_exp& e) {
		cout << "tpool_t " << e.what() << endl;
	}
	try {
		if (num_threads > MAX_THREAD_NUM) {
			debug(TPOOL_ERROR, "too many threads");
			throw cst_exp();
		} catch (cst_exp& e) {
			cout << "tpool_t " << e.what() << endl;
		}
	}
	threads = vector<thread_t>(num, new thread_t());
	schedule_thread = round_robin_schedule;

	if (signal(SIGUSR1, sig_do_nothing) == SIG_ERR) {
		debug(TPOOL_ERROR, "signal failed");
		return NULL;
	}

	main_tid = pthread_self();
	for (int i = 0; i < num_threads; i++) 
		spawn_new_thread(i);
	if (wait_for_thread_registration(num_threads) < 0)
		pthread_exit(NULL);
}

int tpool_t::dispatch_work2thread(int index, void(*routine)(void *), void *arg) {
	tpool_work_t* work = NULL;

	if (thread_queue_full(threads[index])) {
		debug(TPOOL_WARNING, "queue of thread selected is full");
		return -1;
	}
	work = &(threads[index]->work_queue[queue_offset(thread->in)]);
	work->routine = routine;
	work->arg = arg;
	work->next = NULL;
	threads[index]->in++;
	if (thread_queue_len(threads[index] == 1)) {
		debug(TPOOL_DEBUG, "signal has task");
		pthread_kill(threads[index]->id, SIGUSR1);
	}
}

int tpool_t::empty() {
	for (int i = 0; i < num_threads)
		if (!thread_queue_empty(threads[i]))
			return 0;
	return 1;
}

int tpool_t::round_robin_schedule() {
	assert(num_threads > 0);
	cur_thread_index = (cur_thread_index+1) % num_threads;
	return cur_thread_index;
}

int tpool_t::least_load_shcedule() {
	int min_num_works_index = 0;
	assert(num_threads > 0);
	for (int i = 1; i < num_threads; i++) {
		if (thread_queue_len(threads[i]) < thread_queue_len(threads[min_num_works_index]))
			min_num_works_index = i;
	}
	return min_num_works_index;
}

void tpool_t::spawn_new_thread(int index) {
	memset(threads[index], 0, sizeof(thread_t));
	threads[index] = new thread_t();
	if (pthread_create(&threads[index].id, NULL, threads[index].tpool_thread, NULL) != 0) {
		debug(TPOOL_ERROR, "pthread_create failed");
		exit(0);
	}
}

int tpool_t::wait_for_thread_registration(int num_expected) {
	sigset_t signal_mask, oldmask;
	int rc;
	int sig_caught;

	sigemptyset(&oldmask);
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGUSR1);
	rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
	if (rc != 0) {
		debug(TPOOL_ERROR, "SIG_BLOCK failed");
		return -1;
	}

	while (global_num_thread < num_expected) {
		rc = sigwait(&signal_mask, &sig_caught);
		if (rc != 0) {
			debug(TPOOL_ERROR, "sigwait failed");
			return -1;
		}
	}

	rc = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
	if (rc != 0) {
		debug(TPOOL_ERROR, "SIG_SETMASK failed");
		return -1;
	}
	return 0;
}


