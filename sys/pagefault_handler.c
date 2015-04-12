#include <sys/freelist.h>
#include <sys/paging.h>
#include <sys/kmalloc.h>
#include <sys/sbunix.h>
#include <sys/process.h>

extern task_struct_t *currenttask;

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

void get_elf_ptr(char** elf_dptr, mem_desc_t* mem_ptr, int type) {
	if (type == 0)
		*elf_dptr = mem_ptr->text_elf_addr;
	else if (type == 1)
		*elf_dptr = mem_ptr->rodata_elf_addr;
	else if (type == 2)
		*elf_dptr = mem_ptr->data_elf_addr;
}

void do_demand_paging(uint64_t virtual_addr) {
	char * vaddr = (char *) virtual_addr;
	mem_desc_t * mem_ptr = currenttask->mem_map;
	vma_t * temp_vma = mem_ptr->vma_list;

	if (temp_vma != NULL) {
		page_alloc(virtual_addr);
		// copy 1 page worth of stuff from addresses pointed to by vmas
		while (temp_vma != NULL && ((uint64_t) vaddr) < virtual_addr + 4096) {
			if ((uint64_t) vaddr >= temp_vma->vma_start
					&& (uint64_t) vaddr < temp_vma->vma_end) {
				// load elf for this range of linear addr
				if (temp_vma->type >= 0 && temp_vma->type <= 3) {
					// text or rodata or data
					char * elf_ptr = NULL;
					get_elf_ptr(&elf_ptr, mem_ptr, temp_vma->type);
					while ((uint64_t) vaddr < temp_vma->vma_end) {
						*vaddr = *elf_ptr;
						vaddr++;
						elf_ptr++;
					}
				} else if (temp_vma->type == 3){
					// bss section
					while ((uint64_t) vaddr < temp_vma->vma_end) {
						*vaddr = 0;
						vaddr++;
					}
				}
				// nothing to do if type == 4 (stack demand paging),
				// as page has been allocated on top already.
			}
			temp_vma = temp_vma->vma_next;
		}
	} else {
		printf(" ERROR: no proper vma for demand paging\n");
	}

}

void do_page_fault(uint64_t addr) {
	printf("DO PAGE FAULT");
}

void do_handle_pagefault(uint64_t error_code) {
//	printf(", error code is.. %x\n", error_code);
	int present = get_bit(error_code, 0);
	int rw = get_bit(error_code, 1);
	int us = get_bit(error_code, 2);
	uint64_t addr = get_faulted_addr();
	if (present == 0) {
		int kernel_addr = is_kernel_addr(addr);
		if (us == 1) {
			if (kernel_addr) {
				//trying to access kernel data
				printf(" Kernel access by user");
				do_page_fault(addr);
			} else {
				printf(" Demand paging for addr %p", addr);
				do_demand_paging(addr);
			}
		} else {
			printf(" Kernel page fault. Do not reach here unless testing ");
			page_alloc(addr);
		}
	} else {
		printf(" must be illegal access p:rw:us %d:%d:%d", present, rw, us);
		do_page_fault(addr);
	}

}
