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

void seg_fault(uint64_t addr) {
	printf("DO PAGE FAULT\n");
}

void copy_byte_from_apt_elf(char *vaddr, vma_t* temp_vma, mem_desc_t * mem_ptr) {
	char * elf_sec_ptr = NULL;
	get_elf_ptr(&elf_sec_ptr, mem_ptr, temp_vma->type);
	char * elf_ptr = elf_sec_ptr + (vaddr - (char *) temp_vma->vma_start);
	*vaddr = *elf_ptr;
}

int is_addr_valid(uint64_t virtual_addr, mem_desc_t* mem_ptr) {
	int flag = 0;
	for (vma_t* temp_vma = mem_ptr->vma_list; temp_vma != NULL; temp_vma =
			temp_vma->vma_next) {
		if (virtual_addr >= temp_vma->vma_start
				&& virtual_addr < temp_vma->vma_end) {
			flag = 1;
			break;
		}
	}
	return flag;
}

void do_demand_paging(uint64_t virtual_addr) {
	mem_desc_t * mem_ptr = currenttask->mem_map;
	vma_t * temp_vma = mem_ptr->vma_list;

	if (!is_addr_valid(virtual_addr, mem_ptr)) {
		printf("No valid VMAs for this addr %p", virtual_addr);
		seg_fault(virtual_addr);
		return;
	}
	page_alloc(virtual_addr);

	if (temp_vma != NULL) {
		uint64_t page_virtual_addr = virtual_page_base(virtual_addr);
		uint64_t temp = page_virtual_addr;
		while (temp - page_virtual_addr < 4096) {
			for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL;
					temp_vma = temp_vma->vma_next) {
				if (temp >= temp_vma->vma_start && temp < temp_vma->vma_end) {
					if (temp_vma->type >= 0 && temp_vma->type < 3) {
						// text or rodata or data
						copy_byte_from_apt_elf((char *) temp, temp_vma,
								mem_ptr);
					} else if (temp_vma->type == 3) {
						// bss section
						*((char *) temp) = 0;
					}
				}
			}
			temp++;
		}
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
				printf(" Demand paging for process %d for addr %p\n",
						currenttask->pid, addr);
				do_demand_paging(addr);
			}
		} else {
			printf(" Pid:%d, kernel page fault. Do not reach here unless"
					" testing.page fault at %p, error_code: %x  \n",
					currenttask->pid, addr, error_code);
			page_alloc(addr);
		}
	} else {
		printf(" must be illegal access pid %d %p p:rw:us %d:%d:%d\n",
				currenttask->pid, addr, present, rw, us);
		seg_fault(addr);
	}

}
