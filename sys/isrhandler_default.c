#ifndef ISRHANDLER_DEFAULT_C
#define ISRHANDLER_DEFAULT_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include<sys/process.h>
#include <sys/freelist.h>
#include <sys/paging.h>

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
//	printf("trap thirteen");
}
void traphandler_fourteen() {
	printf("trap fourteen");
	uint64_t* frame = get_free_frame();
	uint64_t virtual_addr = 0;
	__asm__ __volatile(
			"movq %%cr2,%0"
			:"=a"(virtual_addr)
			:
	);

	//todo :what if kernel page faults??
	setup_process_page_tables((uint64_t) virtual_addr,
			(uint64_t) frame);

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
void isrhandler_syscall(){
	preempt();
}
#endif
