#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>
//typedef uint64_t size_t;
//typedef uint64_t ssize_t;
//typedef int32_t pid_t;
/*
 static __inline uint64_t syscall_n(int formatc,format,...){
 const char *format;
 int i=0;
 for(for i=0; i<formatc; i++){

 }
 }*/
static __inline uint64_t syscall_0(uint64_t n) {
	uint64_t result;
	__asm__ __volatile(
			"xor %%rbx, %%rbx\n\t"
			"syscall"
			:"=a"(result)
			:"0"(n)
			:"rbx"
	);
	return result;
}

// todo: need to check if this supports return of addresses too.
static __inline uint64_t syscall_1_p(uint64_t n, uint64_t size) {
	uint64_t result;
	__asm__ __volatile(
			//"xor %%rbx, %%rbx\n\t"
			"syscall"
			:"=a"(result)
			:"0"(n),"D"(size));
	return result;
}
static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t result;
	__asm__ __volatile(
			//"movq %0, %%rax\n\t"
			//"movq %1, %%rdi\n\t"
			"syscall"
			:"=&a" (result)
			:"0"(n),"D"(a1));
	return result;
}
/*
//todo:check if works
static __inline uint64_t syscall_2(uint64_t n, const void *a1, int a2) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2));
	return result;

}
*/
static __inline uint64_t syscall_2_test(uint64_t n, uint64_t a1, uint64_t a2) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2));
	return result;

}
// changed constraint to 'r' to make this method generic
// todo: need to check if this supports return of addresses too.
static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2,
		uint64_t a3) {
	uint64_t result;
	__asm__ __volatile__(
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3));
	return result;
}
static __inline pid_t syscall_4_wait(uint64_t n, uint64_t a1, int *a2, int a3) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3));
	return result;
}
static __inline pid_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2), "d"(a3));
	return result;
}
static __inline size_t syscall_4_write(uint64_t n, int a1, const void *a2,
		size_t a3) {
	uint64_t result;
	__asm__ __volatile__ (
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1),"S"(a2),"d"(a3));
	return result;
}

static __inline uint64_t syscall_3_dup2(uint64_t n, int a1, int a2) {
	uint64_t result;
	__asm__ __volatile__(
			"movq $0,%%rcx\n\t"
			"syscall"
			:"=a" (result)
			:"0"(n), "D"(a1), "S"(a2));
	return result;
}

#endif
