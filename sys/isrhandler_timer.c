#ifndef ISRHANDLER_TIMER_H
#define ISRHANDLER_TIMER_H
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
void isrhandler_timer(){
//	__asm__ __volatile__(
//	"movq $0xb8000, %rax\n\t"
//	"movb $69, (%rax)\n\t"
//	"movq $0xb8004, %rax\n\t"
//	"movb $71, (%rax)\n\t"
//	);
	printf("Interupt ");
}

#endif
