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
#endif
