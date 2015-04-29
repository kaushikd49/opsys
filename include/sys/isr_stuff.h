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
	uint64_t gs;//top
	uint64_t fs;//8
	uint64_t es;//16
	uint64_t ds;//24
	uint64_t r15;//32
	uint64_t r14;//40
	uint64_t r13;//48
	uint64_t r12;//56
	uint64_t r11;//64
	uint64_t r10;//72
	uint64_t r9;//80
	uint64_t r8;//88
	uint64_t rbp;//96
	uint64_t rsi;//104
	uint64_t rdi;//112
	uint64_t rdx;//120
	uint64_t rcx;//128
	uint64_t rbx;//136
	uint64_t rax;//144
	uint64_t rip;//152
	uint64_t cs;//160
	uint64_t flag;//168
	uint64_t rsp;//176
	uint64_t ss;//184

} regs_syscall_t;

#endif
