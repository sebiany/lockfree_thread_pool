#ifndef __TPOOL_WORK_H
#define __TPOOL_WORK_H

typedef struct tpool_work {
	void				(*routine)(void *);
	void 				*arg;
	struct tpool_work 	*next;
} tpool_work_t;

#endif