#ifndef __KERNEL_THREAD_H
#define __KERNEL_THREAD_H
#include<sys/defs.h>
void waiting_to_running_q();
void read_thread_process(int fd, char *buffer, uint64_t size);
void clear_keyboard_busy();
void check_user_process_waitpid_daemon();
void return_blocking_rw_to_runq();
void write_thread_process(int fd, void *buffer, uint64_t size);
void clean_up_processes();
#endif
