#ifndef _EXIT_C
#define _EXIT_C
#include <stdarg.h>
#include <sys/defs.h>
#include <sys/syscall.h>
static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t result;
	__asm__ __volatile(
			"int $0x80"
			:"=&a" (result)
			:"0"(n),"D"(a1)
			:"%rbx", "%rcx", "%rdx", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14","%r15");
	return result;
}
void exit(int status) {
	syscall_1(SYS_exit, status);
}
#endif
