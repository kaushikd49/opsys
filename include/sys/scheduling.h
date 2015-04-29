#ifndef _SCHEDULING_H
#define _SCHEDULING_H
#include<sys/defs.h>
void add_process_runq(task_struct_t *runnable_process);
void add_process_waitq(task_struct_t *waiting_process);
void move_process_runq_to_waitq(uint64_t pid);
void move_process_waitq_to_runq(uint64_t pid);

#endif
