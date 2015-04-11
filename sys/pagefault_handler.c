#include <sys/freelist.h>
#include <sys/paging.h>
#include <sys/kmalloc.h>
#include <sys/sbunix.h>

uint64_t get_faulted_addr() {
	uint64_t virtual_addr = 0;
	__asm__ __volatile(
			"movq %%cr2,%0"
			:"=a"(virtual_addr)
			:
	);
	return virtual_addr;
}
void page_alloc(uint64_t virtual_addr) {
	uint64_t* frame = get_free_frame();
	//todo :what if kernel page faults??
	setup_process_page_tables((uint64_t) virtual_addr, (uint64_t) frame);
}

void do_page_fault(uint64_t addr) {
	printf("DO PAGE FAULT");
}

void do_handle_pagefault(uint64_t error_code) {
	printf(", error code is.. %x\n", error_code);
	int present = error_code & 1;
	int rw = error_code & 2;
	int us = error_code & 4;
	uint64_t addr = get_faulted_addr();
	if (present == 0) {
		int kernel_addr = is_kernel_addr(addr);
		if (us == 1) {
			if (kernel_addr) {
				//trying to access kernel data
				do_page_fault(addr);
				printf("Kernel access by user");
			} else {
				page_alloc(addr);
				printf("Allocating frame");
			}
		} else {
			printf(" Kernel page fault. Do not reach here unless testing ");
			page_alloc(addr);
		}
	} else {
		do_page_fault(addr);
		printf("must be illegal access p:rw:us %d:%d:%d", present, rw, us);
	}

}
