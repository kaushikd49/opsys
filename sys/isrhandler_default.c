#ifndef ISRHANDLER_DEFAULT_C
#define ISRHANDLER_DEFAULT_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/process.h>
#include <sys/pagefault_handler.h>
#include <sys/process.h>
#include<sys/system_calls.h>
#include<sys/syscall.h>
#include<sys/tarfs_FS.h>
#include <sys/isr_stuff.h>
typedef struct {
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t error_code;
} regs_pushd_t;

uint64_t handle_exit(regs_syscall_t regs) {
	uint64_t stack_top = (uint64_t) (&(regs.gs));
	return temp_preempt_exit(stack_top);
}

uint64_t handle_syscall(regs_syscall_t regs) {
	if (regs.rax == 1) {
		return write_system_call((int) regs.rdi, (const void *) regs.rsi,
				(size_t) regs.rdx);
	} else if (regs.rax == 57) {
		uint64_t stack_top = (uint64_t) (&(regs.gs));
		return fork_sys_call(stack_top);
	} else if(regs.rax == SYS_write){
		return write_system_call((int)regs.rdi, (const void *)regs.rsi, (size_t)regs.rdx);
	}
	else if(regs.rax == SYS_open){
		return open_tarfs((char *)regs.rdi, (int)regs.rsi);
	}
	else if(regs.rax == SYS_read){
		return read_tarfs((int)regs.rdi, (void *)(regs.rsi), (size_t)regs.rdx);
	}
	else if(regs.rax == SYS_brk){
		return brk_system_call((uint64_t)regs.rdi);
	}
	else if(regs.rax == SYS_getdents){
		dents_tarfs((int)(regs.rdi)	, (struct dirent *)(regs.rsi), (uint64_t)(regs.rdx));
	}
	return 0;
}
void isrhandler_default() {
//	__asm__ __volatile__(
//	"movq $0xb8000, %rax\n\t"
//	"movb $69, (%rax)\n\t"
//	"movq $0xb8004, %rax\n\t"
//	"movb $71, (%rax)\n\t"
//	);
	printf("Interupt occurred, Interrupt handler not mapped");
}
void traphandler_default() {
	printf("trap occurred, trap handler not mapped ");
}
void traphandler_one() {
	printf("trap two");
}
void traphandler_two() {
	printf("trap two");
}

void traphandler_three() {
	printf("trap three");
}
void traphandler_four() {
	printf("trap four");
}
void traphandler_five() {
	printf("trap five");
}
void traphandler_six() {
	printf("trap six");
}
void traphandler_seven() {
	printf("trap seven");
}
void traphandler_eight() {
	printf("trap eight");
}
void traphandler_nine() {
	printf("trap nine");
}

void traphandler_ten() {
	printf("trap ten");
}
void traphandler_eleven() {
	printf("trap eleven");
}
void traphandler_twelve() {
	printf("trap twelve");
}
void traphandler_thirteen() {
	printf("trap thirteen");
}
void traphandler_fourteen(regs_pushd_t regs) {
//	printf("trap fourteen");
	do_handle_pagefault(regs.error_code);
}
void traphandler_fifteen() {
	printf("trap fifteen");
}
void traphandler_sixteen() {
	printf("trap sixteen");
}
void traphandler_seventeen() {
	printf("trap seventeen");
}
void traphandler_eighteen() {
	printf("trap eighteen");
}
void traphandler_nineteen() {
	printf("trap nineteen");
}
void traphandler_twenty() {
	printf("trap twenty");
}
void traphandler_twentyone() {
	printf("trap twentyone");
}
void traphandler_twentytwo() {
	printf("trap twentytwo");
}
void traphandler_twentythree() {
	printf("trap twentythree");
}
void traphandler_twentyfour() {
	printf("trap twentyfour");
}
void traphandler_twentyfive() {
	printf("trap twentyfive");
}
void traphandler_twentysix() {
	printf("trap swentysix");
}
void traphandler_twentyseven() {
	printf("trap twentyseven");
}
void traphandler_twentyeight() {
	printf("trap twentyeight");
}
void traphandler_twentynine() {
	printf("trap twentynine");
}
void traphandler_thirty() {
	printf("trap thirty");
}
void traphandler_thirtyone() {
	printf("trap thirtyone");
}
//void isrhandler_syscall(){
//	preempt();
//}
#endif
