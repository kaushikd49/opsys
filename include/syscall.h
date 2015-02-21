#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

static __inline int64_t syscall_0(uint64_t n) {
	return 0;
}

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
        uint64_t result;
        __asm__ __volatile(
                        "syscall"
                        :"=&a" (result)
                        :"0"(n),"D"(a1));
        return result;
}

static __inline int64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) {
	return 0;
}

static __inline int64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	return 0;
}

#endif
