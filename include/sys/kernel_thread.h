#ifndef __KERNEL_THREAD_H
#define __KERNEL_THREAD_H
#include<sys/defs.h>
void waiting_to_running_q();
void read_thread_process(int fd, void *buffer, uint64_t size);
#endif
