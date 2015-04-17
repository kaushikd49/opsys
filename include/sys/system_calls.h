#ifndef __SYSTEM_CALLS_H
#define __SYSTEM_CALLS_H

#include <sys/defs.h>

int write_system_call(int fd, const void *buff, size_t count);
int fork_sys_call();
#endif
