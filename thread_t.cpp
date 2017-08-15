#include "thread_t.h"

thread_t::thread_t() {
	shutdown = 0;
	in = 0;
	out = 0;
	num_works_done = 0;
}


tpool_work_t* thread_t::get_work_concurrently() {
	tpool_work_t* work = NULL;
	unsigned temp;

	do {
		work = NULL;
		if (thread_queue_len(thread) <= 0)
			break;
		temp = out;
		work = work_queue[queue_offset(temp)];
	} while (!__sync_bool_compare_and_swap(out, temp, temp+1));
	return work;
}

void thread_t::tpool_thread() {
	sigset_t signal_mask, oldmask;
	tpool_work_t *work = NULL;
	int rc;
	int sig_caught;

	__sync_fetch_and_add(&global_num_thread, 1);
	pthread_kill(main_tid, SIGUSR1);

	sigemptyset(&oldmask);
	sigemptyset(&signal_mask);

	sigaddset(&signal_mask, SIGUSR1);

	while (true) {
		rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);
		if (rc != 0) {
			debug(TPOOL_ERROR, "SIG_BLOCK failed");
			pthread_exit(NULL);
		}
		while (thread_queue_empty(this) && !shutdown) {
			debug(TPOOL_DEBUG, "I'm sleeping");
			rc = sigwait(&signal_mask, &sig_caught);
			if (rc != 0) {
				debug(TPOOL_ERROR, "sigwait failed");
				pthread_exit(NULL);
			}
		}

		rc = pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
		if (rc != 0) {
			debug(TPOOL_ERROR, "SIG_SETMASK failed");
			pthread_exit(NULL);
		}
		debug(TPOOL_DEBUG, "I'm awake");

		if (shutdown) {
			debug(TPOOL_DEBUG, "exit");
			debug(TPOOL_INFO, "%ld: %d\n", id, num_works_done);
			pthread_exit(NULL);
		}
		work = get_work_concurrently();
		if (work) {
			thread->num_works_done++;
		}
		if (thread_queue_empty(this))
			pthread_kill(main_tid, SIGUSR1);
	}
}