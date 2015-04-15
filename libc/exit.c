#ifndef _EXIT_C
#define _EXIT_C
#include <stdarg.h>
#include <sys/defs.h>
#include <sys/syscall.h>
static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t result;
	__asm__ __volatile(
			"syscall"
			:"=&a" (result)
			:"0"(n),"D"(a1));
	return result;
}
void exit(int status) {
	syscall_1(SYS_exit, status);
}
#endif
