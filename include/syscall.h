#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

static __inline uint64_t syscall_0(uint64_t n) {
	uint64_t result;
	__asm__ __volatile(
			"xor %%rbx, %%rbx\n\t"
			"int $0x80"
			:"=a"(result)
			:"0"(n)
			 :"%rbx", "%rcx", "%rdx","%rdi","%rsi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"
	);
	return result;
}

// todo: need to check if this supports return of addresses too.
static __inline uint64_t syscall_1_p(uint64_t n, uint64_t size) {
	uint64_t result;
	__asm__ __volatile(
			//"xor %%rbx, %%rbx\n\t"
			"int $0x80"
			:"=a"(result)
			:"0"(n),"D"(size)
			 :"%rbx", "%rcx", "%rdx","%rsi", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
	return result;
}
static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
        uint64_t result;
        __asm__ __volatile(
                        "int $0x80"
                        :"=&a" (result)
                        :"0"(n),"D"(a1));
        return result;
}

static __inline uint64_t syscall_2_test(uint64_t n, uint64_t a1, uint64_t a2) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2)
			:"%rbx", "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15" );
	return result;
}

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2,
		uint64_t a3) {
	uint64_t result;
	__asm__ __volatile__(
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3)
			:"%rbx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15" );
	return result;
}
static __inline pid_t syscall_4_wait(uint64_t n, uint64_t a1, int *a2, int a3) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3)
			:"%rbx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
	return result;
}
static __inline pid_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3)
			 :"%rbx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
	return result;
}
static __inline size_t syscall_4_write(uint64_t n, int a1, const void *a2,
		size_t a3) {
	uint64_t result;
	__asm__ __volatile__ (
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1),"S"(a2),"d"(a3)
			:"%rbx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
	return result;
}

static __inline uint64_t syscall_3_dup2(uint64_t n, int a1, int a2) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"int $0x80"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2)
			:"%rbx", "%rcx", "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15");
	return result;
}

#endif
