#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

enum {
	TPOOL_ERROR,
	TPOOL_WARNING,
	TPOOL_INFO,
	TPOOL_DEBUG
};

#define debug(level, ...) do { \
		if (level < TPOOL_DEBUG) { \
			flockfile(stdout); \
			cout << "###" << pthread_self() << ":" << __func__; \
			cout << __VA__ARGS__ << endl; \
			funlockfile(stdout); \
		} \
} while (0)

#define WORK_QUEUE_POWER 16
#define WORK_QUEUE_SIZE (1 << WORK_QUEUE_POWER)
#define WORK_QUEUE_MASK (WORK_QUEUE_SIZE-1)


#define thread_out_val(thread) 		(__sync_val_compare_and_swap(&(thread)->out, 0, 0))
#define thread_queue_len(thread) 	((thread)->in-thread_out_val(thread))
#define thread_queue_empty(thread) 	((thread_queue_len(thread) == 0))
#define threa_queue_full(thread) 	(thread_queue_len(thread) == WORK_QUEUE_SIZE)
#define queue_offset(val)			((val) & WORK_QUEUE_MASK)

#define MAX_THREAD_NUM				512