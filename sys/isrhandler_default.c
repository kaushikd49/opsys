#ifndef ISRHANDLER_DEFAULT_C
#define ISRHANDLER_DEFAULT_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
void isrhandler_default(){
//	__asm__ __volatile__(
//	"movq $0xb8000, %rax\n\t"
//	"movb $69, (%rax)\n\t"
//	"movq $0xb8004, %rax\n\t"
//	"movb $71, (%rax)\n\t"
//	);
	printf("Interupt occurred, Interrupt handler not mapped");
}
void traphandler_default(){
	printf("trap occurred, trap handler not mapped ");
}
void traphandler_two(){
	printf("trap two");
}
void traphandler_thirteen(){
	printf("trap thirteen");
}
void traphandler_eight(){
	printf("trap eight");
}
#endif
