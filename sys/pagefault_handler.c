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
	uint64_t base_virtual_addr = virtual_addr & (~0xfff);
	setup_process_page_tables(base_virtual_addr, (uint64_t) frame);
}

void get_elf_ptr(char** elf_dptr, mem_desc_t* mem_ptr, int type) {
	if (type == 0)
		*elf_dptr = mem_ptr->text_elf_addr;
	else if (type == 1)
		*elf_dptr = mem_ptr->rodata_elf_addr;
	else if (type == 2)
		*elf_dptr = mem_ptr->data_elf_addr;
}

uint64_t virtual_page_base(uint64_t virtual_addr) {
	return virtual_addr & (~0xfff);
}

char* cp_from_elf(uint64_t page_virtual_addr, vma_t* temp_vma,
		mem_desc_t * mem_ptr) {
	char * vaddr = (char *) temp_vma->vma_start;

	while ((uint64_t) vaddr < temp_vma->vma_end) {
		// only if the vaddr is in the same page as virtual_addr
		if (page_virtual_addr == virtual_page_base((uint64_t) vaddr)) {
			char * elf_sec_ptr = NULL;
			get_elf_ptr(&elf_sec_ptr, mem_ptr, temp_vma->type);
			char * elf_ptr = elf_sec_ptr
					+ (vaddr - (char *) temp_vma->vma_start);
			*vaddr = *elf_ptr;
		}
		vaddr++;
	}
	return vaddr;
}

char* zero_range(uint64_t page_virtual_addr, vma_t* temp_vma) {
	char * vaddr = (char *) temp_vma->vma_start;

	// bss section
	while ((uint64_t) vaddr < temp_vma->vma_end) {
		// only if the vaddr is in the same page as virtual_addr
		if (page_virtual_addr == virtual_page_base((uint64_t) vaddr)) {
			*vaddr = 0;
		}
		vaddr++;
	}
	return vaddr;
}

void seg_fault(uint64_t addr) {
	printf("DO PAGE FAULT\n");
}

void do_demand_paging(uint64_t virtual_addr) {

	mem_desc_t * mem_ptr = currenttask->mem_map;
	vma_t * temp_vma = mem_ptr->vma_list;
	int flag = 0;

	if (temp_vma != NULL) {
		uint64_t page_virtual_addr = virtual_page_base(virtual_addr);

		// copy 1 page worth of stuff from addresses pointed to by vmas
		while (temp_vma != NULL) {
			if (virtual_addr >= temp_vma->vma_start
					&& virtual_addr < temp_vma->vma_end) {

				if (!flag) {
					page_alloc(virtual_addr);
					flag = 1;
				}

				// load elf for this range of linear addr
				if ((temp_vma->type >= 0 && temp_vma->type < 3) ||(temp_vma->type >= 6 && temp_vma->type <= 8)) {
					// text or rodata or data
					cp_from_elf(page_virtual_addr, temp_vma, mem_ptr);
				} else if (temp_vma->type == 3) {
					// bss section
					zero_range(page_virtual_addr, temp_vma);
				}
				// nothing to do if type == 4 (stack demand paging),
				// as page has been allocated on top already.
			}
			temp_vma = temp_vma->vma_next;
		}
	} else {
		printf(" ERROR: no proper vma for demand paging\n");
	}

	// no vmas contain this address
	if (!flag) {
		printf("No VMAs in this range");
		seg_fault(virtual_addr);
	}

}

void do_handle_pagefault(uint64_t error_code) {
	int present = get_bit(error_code, 0);
	int rw = get_bit(error_code, 1);
	int us = get_bit(error_code, 2);
	uint64_t addr = get_faulted_addr();
//	printf(" page fault at %p, error_code: %x ", addr, error_code);
	if (present == 0) {
		int kernel_addr = is_kernel_addr(addr);
		if (us == 1) {
			if (kernel_addr) {
				//trying to access kernel data
				printf(" Kernel access by user\n");
				seg_fault(addr);
			} else {
				printf(" Demand paging for addr %p\n", addr);
				do_demand_paging(addr);
			}
		} else {
			printf(" Kernel page fault. Do not reach here unless"
					" testing.page fault at %p, error_code: %x  \n", addr,
					error_code);
			page_alloc(addr);
		}
	} else {
		printf(" must be illegal access p:rw:us %d:%d:%d\n", present, rw, us);
		seg_fault(addr);
	}

}
