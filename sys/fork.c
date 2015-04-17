#include<sys/paging.h>
#include <sys/kmalloc.h>
#include <sys/sbunix.h>
#include <sys/process.h>

extern task_struct_t *currenttask;

void cp_prev_next(task_struct_t* from, task_struct_t* to) {
	// TODO : below
	//	*next;
	//	*prev;
}

void cp_executable(task_struct_t * from, task_struct_t * to) {
	char *q = to->executable;
	char *p = from->executable;
	while (*p) {
		*q = *p;
		p++;
		q++;
	}
}

void cp_pstate(task_struct_t * from, task_struct_t * to) {
	process_state fstate = from->state;
	process_state tstate = to->state;

	tstate.rsp = fstate.rsp;
	tstate.rip = fstate.rip;
	//general registers
	tstate.rax = fstate.rax;
	tstate.rbx = fstate.rbx;
	tstate.rcx = fstate.rcx;
	tstate.rdx = fstate.rdx;
	tstate.rdi = fstate.rdi;
	tstate.rsi = fstate.rsi;
	tstate.rbp = fstate.rbp;
	tstate.r8 = fstate.r8;
	tstate.r9 = fstate.r9;
	tstate.r10 = fstate.r10;
	tstate.r11 = fstate.r11;
	tstate.r12 = fstate.r12;
	tstate.r13 = fstate.r13;
	tstate.r14 = fstate.r14;
	tstate.r15 = fstate.r15;

	//not copying cr3

	//flags
	tstate.flags = fstate.flags;
	//segment registers
	tstate.cs = fstate.cs;
	tstate.ds = fstate.ds;
	tstate.es = fstate.es;
	tstate.fs = fstate.fs;
	tstate.gs = fstate.gs;
	tstate.ss = fstate.ss;
	tstate.kernel_rsp = tstate.kernel_rsp;
}

void cp_vma(vma_t* from, vma_t* to, mem_desc_t *mem_map) {
	to->my_mem_desc = mem_map;
	to->vma_start = from->vma_start;
	to->vma_end = from->vma_end;
	to->vm_perm = from->vm_perm;
	to->type = from->type;
}

vma_t* cp_linked_vmas(vma_t* from_vma, mem_desc_t* to) {
	vma_t* prev = NULL;
	vma_t* head = NULL;
	while (from_vma != NULL) {
		vma_t* vma = kmalloc(sizeof(vma_t));
		cp_vma(from_vma, vma, to);

		if (prev != NULL) {
			// not first
			prev->vma_next = vma;
		} else {
			// first
			head = vma;
		}

		from_vma = from_vma->vma_next;
		prev = vma;
	}
	return head;
}

void cp_vma_list(mem_desc_t * from, mem_desc_t * to) {
	vma_t *from_vma = from->vma_list;
	vma_t* vma_head = cp_linked_vmas(from_vma, to);
	to->vma_list = vma_head;
}

void cp_vma_cache(mem_desc_t * from, mem_desc_t * to) {
	vma_t *from_vma = from->vma_cache;
	vma_t* vma_head = cp_linked_vmas(from_vma, to);
	to->vma_cache = vma_head;
}

void cp_mem_desc(task_struct_t * from, task_struct_t * to) {
	mem_desc_t *tmem_map = kmalloc(sizeof(mem_desc_t));
	to->mem_map = tmem_map;
	mem_desc_t *fmem_map = from->mem_map;

	cp_vma_list(fmem_map, tmem_map);
	cp_vma_cache(fmem_map, tmem_map);

	tmem_map->mm_users = fmem_map->mm_users;
	tmem_map->mm_count = fmem_map->mm_count;
	tmem_map->num_vma = fmem_map->num_vma;

	//todo: below
//	tmem_map->mem_desc_head = fmem_map->

	tmem_map->text_elf_addr = fmem_map->text_elf_addr;
	tmem_map->rodata_elf_addr = fmem_map->rodata_elf_addr;
	tmem_map->data_elf_addr = fmem_map->data_elf_addr;

	tmem_map->start_code = fmem_map->start_code;
	tmem_map->end_code = fmem_map->end_code;
	tmem_map->start_data = fmem_map->start_data;
	tmem_map->end_data = fmem_map->end_data;
	tmem_map->start_brk = fmem_map->start_brk;
	tmem_map->brk = fmem_map->brk;
	tmem_map->start_stack = fmem_map->start_stack;
	tmem_map->arg_start = fmem_map->arg_start;
	tmem_map->arg_end = fmem_map->arg_end;
	tmem_map->env_start = fmem_map->env_start;
	tmem_map->env_end = fmem_map->env_end;
}

// pte which contains the page that contains
// virtual_addr is marked as read only
void set_pte_as_read(uint64_t virtual_addr) {
	uint64_t * pte = virtual_addr_pte(virtual_addr);
	*pte = set_bit(*pte, 1, 0); // bit 1 as 0
	invalidate_addresses_with_page((uint64_t *)virtual_addr);
}

// mark pages of parent process as read only
void mark_pages_read(task_struct_t * from) {
	mem_desc_t *fmem_map = from->mem_map;
	vma_t *vma = fmem_map->vma_list;
	while (vma != NULL) {
		for (uint64_t addr = vma->vma_start; addr < vma->vma_end; addr +=
				4096) {
			set_pte_as_read(addr);
		}
		vma = vma->vma_next;
	}
}

// reverse mapping of addrs of page table pages
// need this since parent process has to create
// page tables of child without switching cr3
typedef struct phys_virt_mapping {
	uint64_t physical_addr;
	uint64_t virtual_addr;
	struct phys_virt_mapping * next;
} pv_map_t;

//// lookup virtual addr for physical addr
//int phys_to_virt(uint64_t phys_addr, pv_map_t * pv_node, uint64_t *vaddr) {
//	if (pv_node == NULL) {
//		return 0;
//	} else {
//		pv_map_t *temp = pv_node;
//		while (temp != NULL) {
//			if (temp->physical_addr == phys_addr) {
//				*vaddr = temp->virtual_addr;
//				return 1;
//			}
//			temp = temp->next;
//		}
//		return 0;
//	}
//
//}
//
//void insert_into_map(pv_map_t* pv_map_node, uint64_t phys, uint64_t virt) {
//	if(*pv_map_node == NULL){
//		*pv_map_node = kmalloc()
//	}
//}

uint64_t* next_entity_virtual_base(uint64_t* entity_entry, void * pv_map) {
	uint64_t next_entity_base_phys = (uint64_t) next_entity_base(entity_entry,
	NULL);
//	uint64_t vaddr = NULL;
//	int is_found = phys_to_virt(next_entity_base_phys, (pv_map_t) pv_map,
//			&vaddr);
//	if (is_found) {
//		return (uint64_t*) *vaddr;
//	} else {
//		// map the phys addr to a virtual one and cache it in pv_map
//		uint64_t res_addr = (uint64_t) get_virtual_location_cr3(0);
//		insert_into_map(pv_map, next_entity_base_phys, res_addr);
//		return res_addr;
//	}
	uint64_t res_addr = (uint64_t) get_virtual_location(0);
	setup_page_tables_after_cr3_update(res_addr, next_entity_base_phys, 1, 1,
			0);
	// remember to give back this virtual address
	return (uint64_t *)res_addr;
}

// page tables that are involved in resolving page_base are
// created if not already created and entries are duplicated
void cp_ptables_for(uint64_t page_base, pv_map_t* pv_map_node,
		uint64_t ** chld_pml4_base_dbl_ptr) {
	// parent process has not touched this addr, so why should we bother
	// creating page table entry for this in the child? Yes, we don't.
	if (!is_linear_addr_mapped(page_base)) {
		return;
	}
	// store this pte val for the addr page_base in the child page table too
	uint64_t * target_pte = virtual_addr_pte(page_base);
	uint64_t page_phys_addr = (uint64_t) next_entity_base(target_pte, NULL);

	// next_entity_virtual_base is a function similar to
	// next_entity_base in paging.c, except this one returns
	// a virtual address of the next entity base, as once cr3
	// is updated, we need virtual addr to acces any location
	// remaining functionality is identical to that of setting
	// page table before cr3 update - climb down others' page tables
	// and update page table entries.

	// since it is COW, page is marked present, read only and user mode
	setup_page_table_from_outside(page_base, page_phys_addr, 1, 0, 1,
			chld_pml4_base_dbl_ptr, next_entity_virtual_base, pv_map_node);

}

void cp_page_tables(task_struct_t * from, task_struct_t * to) {
	mem_desc_t *fmem_map = from->mem_map;
	vma_t* vma = fmem_map->vma_list;

	uint64_t child_pml_virtual = 0;
	uint64_t child_pml_phys = 1;
	get_both_pml4_base_addrs_for_process(&child_pml_virtual, &child_pml_phys);
	to->state.cr3 = child_pml_phys;

	uint64_t * child_pml_virtual_ptr = (uint64_t *) child_pml_virtual;
	pv_map_t *pv_map_node = NULL; // useless for now
	while (vma != NULL) {
		for (uint64_t addr = vma->vma_start; addr < vma->vma_end; addr +=
				4096) {
			uint64_t page_base = addr & (~0xfff);
			cp_ptables_for(page_base, pv_map_node, &child_pml_virtual_ptr);
		}
	}

}

void copy_tsk(uint64_t pid, task_struct_t * from, task_struct_t * to) {
	to->pid = pid;
	to->ppid = from->pid;
	to->state = from->state;
	cp_prev_next(from, to);
	cp_executable(from, to);
	cp_pstate(from, to);
	cp_mem_desc(from, to);
	mark_pages_read(from);
	cp_page_tables(from, to);
}

void copy_process(uint64_t pid) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	copy_tsk(pid, currenttask, task);
}

void do_fork() {
	uint64_t pid = get_next_pid();
	copy_process(pid);
}

