//#include<sys/paging.h>
//#include <sys/kmalloc.h>
//#include <sys/sbunix.h>
//#include <sys/process.h>
//
//extern task_struct_t *currenttask;
//
//void cp_prev_next(task_struct_t* from, task_struct_t* to) {
//	// TODO : below
//	//	*next;
//	//	*prev;
//}
//
//void cp_executable(task_struct_t * from, task_struct_t * to) {
//	char *q = to->executable;
//	char *p = from->executable;
//	while (*p) {
//		*q = *p;
//		p++;
//		q++;
//	}
//}
//
//void cp_pstate(task_struct_t * from, task_struct_t * to) {
//	fstate = from->process_state;
//	tstate = to->process_state;
//
//	tstate.rsp = fstate.rsp;
//	tstate.rip = fstate.rip;
//	//general registers
//	tstate.rax = fstate.rax;
//	tstate.rbx = fstate.rbx;
//	tstate.rcx = fstate.rcx;
//	tstate.rdx = fstate.rdx;
//	tstate.rdi = fstate.rdi;
//	tstate.rsi = fstate.rsi;
//	tstate.rbp = fstate.rbp;
//	tstate.r8 = fstate.r8;
//	tstate.r9 = fstate.r9;
//	tstate.r10 = fstate.r10;
//	tstate.r11 = fstate.r11;
//	tstate.r12 = fstate.r12;
//	tstate.r13 = fstate.r13;
//	tstate.r14 = fstate.r14;
//	tstate.r15 = fstate.r15;
//
//	//not copying cr3
//
//	//flags
//	tstate.flags = fstate.flags;
//	//segment registers
//	tstate.cs = fstate.cs;
//	tstate.ds = fstate.ds;
//	tstate.es = fstate.es;
//	tstate.fs = fstate.fs;
//	tstate.gs = fstate.gs;
//	tstate.ss = fstate.ss;
//	tstate.kernel_rsp = tstate.kernel_rsp;
//}
//
//void cp_vma(vma_t* from, vma_t* to, mem_desc_t *mem_map) {
//	to->my_mem_desc = mem_map;
//	to->vma_start = from->vma_start;
//	to->vma_end = from->vma_end;
//	to->vm_perm = from->vm_perm;
//	to->type = from->type;
//}
//
//vma_t* cp_linked_vmas(vma_t* from_vma, mem_desc_t* to) {
//	vma_t* prev = NULL;
//	vma_t* head = NULL;
//	while (from_vma != NULL) {
//		vma_t* vma = kmalloc(sizeof(vma_t));
//		cp_vma(from_vma, vma, to);
//
//		if (prev != NULL) {
//			prev->next = vma;
//		} else {
//			head = vma;
//		}
//
//		from_vma = from_vma->next;
//		prev = vma;
//	}
//	return head;
//}
//
//void cp_vma_list(mem_desc_t * from, mem_desc_t * to) {
//	vma_t *from_vma = from->vma_list;
//	vma_t* vma_head = cp_linked_vmas(from_vma, to);
//	to->vma_list = vma_head;
//}
//
//void cp_vma_cache(task_struct_t * from, task_struct_t * to) {
//	vma_t *from_vma = from->vma_cache;
//	vma_t* vma_head = cp_linked_vmas(from_vma, to);
//	to->vma_cache = vma_head;
//}
//
//void cp_mem_desc(task_struct_t * from, task_struct_t * to) {
//	mem_desc_t *tmem_map = kmalloc(sizeof(mem_desc_t));
//	mem_desc_t *fmem_map = from->mem_map;
//
//	cp_vma_list(fmem_map, tmem_map);
//	cp_vma_cache(fmem_map, tmem_map);
//
//	tmem_map->mm_users = fmem_map->mm_users;
//	tmem_map->mm_count = fmem_map->mm_count;
//	tmem_map->num_vma = fmem_map->num_vma;
//
//	//todo: below
////	tmem_map->mem_desc_head = fmem_map->
//
//	tmem_map->text_elf_addr = fmem_map->text_elf_addr;
//	tmem_map->rodata_elf_addr = fmem_map->rodata_elf_addr;
//	tmem_map->data_elf_addr = fmem_map->data_elf_addr;
//
//	tmem_map->start_code = fmem_map->start_code;
//	tmem_map->end_code = fmem_map->end_code;
//	tmem_map->start_data = fmem_map->start_data;
//	tmem_map->end_data = fmem_map->end_data;
//	tmem_map->start_brk = fmem_map->start_brk;
//	tmem_map->brk = fmem_map->brk;
//	tmem_map->start_stack = fmem_map->start_stack;
//	tmem_map->arg_start = fmem_map->arg_start;
//	tmem_map->arg_end = fmem_map->arg_end;
//	tmem_map->env_start = fmem_map->env_start;
//	tmem_map->env_end = fmem_map->env_end;
//}
//
//// pte which contains the page that contains
//// virtual_addr is marked as read only
//void set_pte_as_read(uint64_t virtual_addr) {
//	uint64_t * pte = virtual_addr_pte(virtual_addr);
//	*pte = set_bit(*pte, 1, 0); // bit 1 as 0
//	invalidate_addresses_with_page(virtual_addr);
//}
//
//void mark_pages_read(task_struct_t * from) {
//	mem_desc_t *fmem_map = from->mem_map;
//	vma_t vma = fmem_map->vma_list;
//	while (vma != NULL) {
//		for (uint64_t addr = vma->vma_start; addr < vma->vma_end; addr +=
//				4096) {
//			set_pte_as_read(addr);
//			num_pages++;
//		}
//		vma = vma->vma_next;
//	}
//	return num_pages;
//}
//
//// reverse mapping of addrs of page table pages
//// need this since parent process has to create
//// page tables of child without switching cr3
//typedef struct phys_virt_mapping {
//	uint64_t physical_addr;
//	uint64_t virtual_addr;
//	phys_virt_mapping * next;
//} pv_map_t;
//
//// page tables that are involved in resolving page_base are
//// created if not already created and entries are duplicated
//void cp_ptables_for(uint64_t page_base, pv_map_t* pv_map_node,
//		uint64_t * chld_pml4_base) {
//	// parent process has not touched this addr, so why should we bother
//	// creating page table entry for this in the child? Yes, we don't.
//	if (!is_linear_addr_mapped(page_base)) {
//		return;
//	}
//	// store this pte val for the addr page_base in the child page table too
//	uint64_t * target_pte = virtual_addr_pte(page_base);
//	uint64_t page_phys_addr  = (uint64_t)next_entity_base(target_pte)
//
//
//	uint64_t pml4_offset = extract_bits(page_base, 39, 47, ULONG_ZERO, 0, 8);
//	if (is_entry_not_created(chld_pml4_base + pml4_offset)) {
//
//	}
//
//}
//
//void cp_page_tables(task_struct_t * from, task_struct_t * to) {
//	mem_desc_t *fmem_map = from->mem_map;
//	vma_t vma = fmem_map->vma_list;
//	uint64_t * chld_pml4_base = kmalloc(4096);
//	pv_map_t pv_map_node = NULL;
//
//	while (vma != NULL) {
//		for (uint64_t addr = vma->vma_start; addr < vma->vma_end; addr +=
//				4096) {
//			uint64_t page_base = addr & (~0xfff);
//			cp_ptables_for(page_base, &pv_map_node, chld_pml4_base);
//		}
//
//	}
//
//}
//
//void copy_tsk(uint64_t, pid, task_struct_t * from, task_struct_t * to) {
//	to->pid = pid;
//	to->ppid = from->pid;
//	to->state = from->state;
//	cp_prev_next(from, to);
//	cp_executable(from, to)
//	cp_pstate(from, to);
//	cp_mem_desc(from, to);
//	mark_pages_read(from);
//	cp_page_tables(from, to);
//}
//
//void copy_process(uint64_t pid) {
//	task_struct_t *task = kmalloc(sizeof(task_struct_t));
//	copy_tsk(pid, currenttask, task);
//
//}
//
//void do_fork() {
//	uint64_t pid = get_next_pid();
//	copy_process(pid);
//}
//
