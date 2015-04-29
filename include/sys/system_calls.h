#ifndef __SYSTEM_CALLS_H
#define __SYSTEM_CALLS_H
#include<sys/nanosleep_functions.h>
#include <sys/defs.h>

int write_system_call(int fd, const void *buff, size_t count);
int fork_sys_call();
uint64_t brk_system_call(uint64_t value);
uint64_t wait_pid(int pid, int *status, int options, uint64_t stack_top);
uint64_t nanosleep_sys_call(const struct timespec *rqtp, struct timespec *rmtp, uint64_t stack_top);
uint64_t execve_sys_call(char *binary, char **argv, char **envp, uint64_t stack_top);
#endif
