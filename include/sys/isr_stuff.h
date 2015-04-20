#ifndef _ISR_STUFF_H
#define _ISR_STUFF_H
#include<sys/defs.h>
extern void keyboard_init();
extern void init_keyboard_map();
int printHexIntTime(int n);
void add_custom_interrupt();
void add_int_handler(uint64_t isr_base, uint64_t isr_number, uint64_t handler_name, char type, uint16_t segment_selector);
void init_IDT(struct lidtr_t IDT);
void init_init_IDT();
void config_PIC();

typedef struct {
	uint64_t gs;
	uint64_t fs;
	uint64_t es;
	uint64_t ds;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
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
	uint64_t rip;
	uint64_t cs;
	uint64_t flag;
	uint64_t rsp;
	uint64_t ss;

} regs_syscall_t;

#endif
