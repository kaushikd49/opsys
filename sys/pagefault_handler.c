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
	else if (type == 6)
//		ehframe_elf_addr;
//			char *got_elf_addr;
//			char *gotplt_elf_addr;
		*elf_dptr = mem_ptr->ehframe_elf_addr;
	else if (type == 7)
		*elf_dptr = mem_ptr->got_elf_addr;
	else if (type == 8)
		*elf_dptr = mem_ptr->gotplt_elf_addr;
}

uint64_t virtual_page_base(uint64_t virtual_addr) {
	return virtual_addr & (~0xfff);
}

void copy_byte_from_apt_elf(char *vaddr, vma_t* temp_vma, mem_desc_t * mem_ptr) {
	char * elf_sec_ptr = NULL;
	get_elf_ptr(&elf_sec_ptr, mem_ptr, temp_vma->type);
	char * elf_ptr = elf_sec_ptr + (vaddr - (char *) temp_vma->vma_start);
	*vaddr = *elf_ptr;
}

void seg_fault(uint64_t addr) {
	printf("DO PAGE FAULT\n");
}

int is_addr_in_vma(uint64_t virtual_addr, mem_desc_t* mem_ptr, uint64_t *rsp_val) {
	int flag = 0;
	for (vma_t* temp_vma = mem_ptr->vma_list; temp_vma != NULL; temp_vma =
			temp_vma->vma_next) {
		if (virtual_addr >= temp_vma->vma_start
				&& virtual_addr < temp_vma->vma_end) {
			flag = 1;
			break;
		}
	}
//	printf("rspval:%x", rsp_val);
	if(virtual_addr >= (uint64_t)rsp_val-8){
		flag = 1;
	}
	return flag;
}

int is_addr_writable_in_vma(uint64_t virtual_addr, mem_desc_t* mem_ptr) {
	int flag = 0;
	for (vma_t* temp_vma = mem_ptr->vma_list; temp_vma != NULL; temp_vma =
			temp_vma->vma_next) {
		if (virtual_addr >= temp_vma->vma_start
				&& virtual_addr < temp_vma->vma_end
//				&& vma->vma_perm == 1
						) {
			flag = 1;
			break;
		}
	}
	return flag;
}
inline uint64_t max(uint64_t a, uint64_t b){
	return a>b?a:b;
}
void do_demand_paging(uint64_t virtual_addr, uint64_t *rsp_val) {
	mem_desc_t * mem_ptr = currenttask->mem_map;
	vma_t * temp_vma = mem_ptr->vma_list;

	if (!is_addr_in_vma(virtual_addr, mem_ptr, rsp_val)) {
		printf("No valid VMAs for this addr %p", virtual_addr);
		seg_fault(virtual_addr);
		return;
	}
	uint64_t heap_end = 0;
	if(virtual_addr >= (uint64_t)rsp_val-8){
		for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL;
				temp_vma = temp_vma->vma_next){
			if(temp_vma->type == 5){
				heap_end =temp_vma->vma_end;
			}
		}
		for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL;
				temp_vma = temp_vma->vma_next){
			if(temp_vma->type == 4){
				if(virtual_addr <=heap_end){
					seg_fault(virtual_addr);
				}
				else{
					uint64_t page_virtual_addr = virtual_page_base(virtual_addr);
					temp_vma->vma_start = max(heap_end, page_virtual_addr);
				}
			}
		}
	}
	page_alloc(virtual_addr);

	if (temp_vma != NULL) {
		uint64_t page_virtual_addr = virtual_page_base(virtual_addr);
		uint64_t temp = page_virtual_addr;
		while (temp - page_virtual_addr < 4096) {
			for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL;
					temp_vma = temp_vma->vma_next) {
				if (temp >= temp_vma->vma_start && temp < temp_vma->vma_end) {
					if ((temp_vma->type >= 0 && temp_vma->type < 3)
							|| (temp_vma->type >= 6 && temp_vma->type < 8)) {
						// text or rodata or data
						copy_byte_from_apt_elf((char *) temp, temp_vma,
								mem_ptr);
					} else if (temp_vma->type == 3) {
						// bss section
						*((char *) temp) = 0;
					} else if (temp_vma->type == 5) {
						*((char *) temp) = 0;
					}
					else if(temp_vma->type == 20){
						*((char *)temp) = 0;
					}
					else if(temp_vma->type == 4){
						*((char *)temp) =0;
					}
				}

			}
			temp++;
		}
	}
}

void bad_kernel_access(uint64_t addr) {
	//trying to access kernel data
	printf(" Kernel access by user\n");
	seg_fault(addr);
}

int user_access(int us) {
	return us == 1;
}

void set_as_write(uint64_t* pte) {
	*pte = set_bit(*pte, 1, 1); // bit 1 as 1
}

// will do cr3 refresh here, which is simpler than
// tlb flush for all possible addrs
void set_pagetables_as_write(uint64_t virtual_addr) {
	if (is_linear_addr_mapped(virtual_addr)) {
		set_as_write(virtual_addr_pml4e(virtual_addr));
		set_as_write(virtual_addr_pdirptre(virtual_addr));
		set_as_write(virtual_addr_pdire(virtual_addr));
		set_as_write(virtual_addr_pte(virtual_addr));
	}
	update_cr3((uint64_t *) currenttask->state.cr3);
}

uint64_t* duplicate_page(uint64_t addr) {
	// alloc separate page and copy, decr ref count
	uint64_t* temp_addr = get_virtual_location(0);
	uint64_t* frame = get_free_frame();
	setup_process_page_tables((uint64_t) temp_addr, (uint64_t) frame);
	uint64_t* p = (uint64_t*) (addr & (~0xfff));
	for (int i = 0; i < 512; i++) {
		*temp_addr = *p;
		temp_addr++;
		p++;
	}
	return frame;
}

int is_cow_possible(uint64_t addr) {
	mem_desc_t * mem_ptr = currenttask->mem_map;

	if (!is_addr_writable_in_vma(addr, mem_ptr)) {
		printf("No write perm in VMAs for this addr %p", addr);
		seg_fault(addr);
		return 0;
	}
	return 1;
}

int copy_on_write(uint64_t addr) {
	// addr is a valid one that has write
	// permission in vma, so lets perform COW
	uint64_t phys = phys_addr_of_frame(addr);
	printf("ref count is %d ", get_ref_count(phys));

	if (get_ref_count(phys) == 1) {
		// just mark as writable
		printf(" just made page writable ");
		set_pagetables_as_write(addr);
	} else {
		// alloc separate page and copy, decr ref count
		uint64_t* frame = duplicate_page(addr);
		setup_process_page_tables_without_zeroing(addr, (uint64_t) frame);
		decrease_ref_count(phys);
		printf(" allocated a new page and copied contents ");
	}
	return 1;
}

void do_handle_pagefault(uint64_t error_code, uint64_t *rsp_val) {
	int present = get_bit(error_code, 0);
	int rw = get_bit(error_code, 1);
	int us = get_bit(error_code, 2);
	uint64_t addr = get_faulted_addr();
	int kernel_addr = is_kernel_addr(addr);
//	printf(" pid:%d page fault at %p, error_code: %x ", currenttask->pid, addr,
//			error_code);
	if (present == 0) {
		if (user_access(us)) {
			if (kernel_addr) {
				bad_kernel_access(addr);
			} else {
//				printf(" Demand paging for process %d for addr %p\n",
//						currenttask->pid, addr);
				do_demand_paging(addr, rsp_val);
			}
		} else {
			printf(" Pid:%d, kernel page fault. Do not reach here unless"
					" testing.page fault at %p, error_code: %x  \n",
					currenttask->pid, addr, error_code);
			page_alloc(addr);
		}
	} else {
		if (user_access(us)) {
			if (kernel_addr) {
				bad_kernel_access(addr);
			} else if (is_cow_possible(addr)) {
				printf(" Performing COW ");
				copy_on_write(addr);
			} else {
				printf("\nProcess %d trying to write into protected area %p ",
						currenttask->pid, addr);
				seg_fault(addr);
			}
		} else {
			printf(" Something wrong, kernel read fault pid:%d at %p p:rw:us %d:%d:%d\n",
					currenttask->pid, addr, present, rw, us);
			seg_fault(addr);
		}
	}
}
