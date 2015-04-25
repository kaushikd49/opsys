#ifndef __NANOSLEEP_FUNCTION_C
#define __NANOSLEEP_FUNCTION_C
#include<sys/defs.h>
#include<sys/process.h>
#include<sys/kmalloc.h>
#include<sys/printtime.h>
	struct timespec {
	time_t tv_sec;
	long tv_nsec;
	};
	struct nanosleep_node{
		uint64_t seconds;
		uint64_t ms;
		task_struct_t *task;
		struct nanosleep_node *next;
	};
	typedef struct nanosleep_node nanosleep_node_t;
	extern nanosleep_node_t *nanosleep_head;
	nanosleep_node_t *make_nanosleep_node(const struct timespec *rqtp,task_struct_t *task);
	void add_nanosleep_list(nanosleep_node_t *node);
	void remove_nanosleep_list(uint64_t seconds, uint64_t ms);
	uint64_t temp_preempt_nanosleep(const struct timespec *rqtp, struct timespec *rmtp, uint64_t stack_top);
#endif
