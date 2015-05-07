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
int check_ready_read(file_desc_t *current_fd);
int read_stdin(int fd, uint64_t size, char* buffer);
void block_read_signal_write(pipe_struct_t* pipe_info);
#endif
