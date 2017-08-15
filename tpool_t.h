#ifndef __TPOOL_T_H
#define	__TPOOL_T_H
#include "thread_t.h"


class tpool_t {
public:
	tpool_t(int num_threads);

	int empty();
	void spawn_new_thread(int index);
	int wait_for_thread_registration(int num_expected);
	int dispatch_work2thread(int index, void(*routine)(void *), void *arg);

	static int round_robin_schedule();
	static int least_load_schedule();

private:
	static int 			cur_thread_index;

	int					num_threads;
	vector<thread_t*>	threads;

};

int tpool_t::cur_thread_index = -1;

#endif