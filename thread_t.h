#ifndef __THREAD_T_H
#define __THREAD_T_H

#include "conf.h"
#include "tpool_work_t"

class thread_t {
public:
	thread_t();

	tpool_word_t* get_work_concurrently();
	void tpool_thread();

private:
	pthread_t	id;
	int 		shutdown;

	int 		num_works_done;
	unsigned	in;
	unsigned	out;
	tpool_work_t work_queue[WORK_QUEUE_SIZE];
};




#endif