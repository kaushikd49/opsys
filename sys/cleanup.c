#include <sys/paging.h>
#include <sys/freelist.h>
#include <sys/kmalloc.h>
#include <sys/utils.h>
#include <sys/process.h>
#include <sys/sbunix.h>
#include <sys/freelist.h>
void free_vma(vma_t* q) {
	if (q != NULL) {
		kfree(q);
	}
}

void cleanup_vmas(vma_t * vma) {
	vma_t * p = vma;
	vma_t * q = NULL;
	while (p != NULL) {
		free_vma(q);
		q = p;
		p = p->vma_next;
	}
	free_vma(q);
}
void space_msg() {
	printf(" #zeroed free-pages %d \n", get_zerod_pages_count());
	printf(" #tot free-pages %d \n", get_unused_pages_count());
}

void cleanup_mem_map(task_struct_t * task) {
	mem_desc_t * mem_map = task->mem_map;
	cleanup_vmas(mem_map->vma_list);
	cleanup_vmas(mem_map->vma_cache);
	kfree(mem_map);
}

//void cleanup_fds(task_struct_t * task) {
//	// todo: this. also, need to keep the ptr for beginning of buffer
//
//	for (uint64_t i = 0; i < MAX_NUMBER_FILES; i++) {
//		if (task->filearray[i] != NULL) {
//			decrement_global_count_fd(task->filearray[i]);
//			task->filearray[i] = NULL;
//		}
//	}
////	kfree(task->filearray);
//}

void free_frame(task_struct_t * task, uint64_t phys_addr, uint64_t vaddr) {
//	printf(" freeing actual frame %p \n", phys_addr);
	decrease_ref_count(phys_addr);
}

uint64_t get_phys_from_virt_of_other_process(uint64_t *pml4_base_virt,
		uint64_t vaddr, pv_map_t* pv_map_node) {

	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t* deepest_entity = &abc;
	uint64_t* deepest_entity_base = &def;

	int res = page_lookup(pml4_base_virt, vaddr, deepest_entity,
			deepest_entity_base, phys_to_virt_map, pv_map_node);

	uint64_t phys_addr = (uint64_t) next_entity_base(
			(uint64_t *) *deepest_entity);

	if (res != 0) {
		return 0;
	} else {
		return phys_addr;
	}

}

void free_if_not_freed(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t vaddr, uint64_t page_phys_addr) {
	if (if_not_contains_phys_addr(pv_map_node, vaddr, page_phys_addr)) {
		free_frame(task, page_phys_addr, vaddr);
		cache_pv_mapping(pv_map_node, vaddr, page_phys_addr);
	}
}

void free_process_page_for_process_virt_addr(task_struct_t * task,
		uint64_t* pml_virt, uint64_t vaddr, pv_map_t* pv_map_node) {
	// actual physical address of the page vaddr for the process with cr3 task->state.cr3
	uint64_t page_phys_addr = get_phys_from_virt_of_other_process(pml_virt,
			vaddr, pv_map_node);
	if (page_phys_addr != 0) {
		// physical page present in process
		free_if_not_freed(task, pv_map_node, vaddr, page_phys_addr);
	} else {
//		printf(" not freaable page %p \n", page_phys_addr);
	}
}

void cleanup_process_pages(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t* pml_virt) {
	mem_desc_t * mem_map = task->mem_map;
	vma_t * vma = mem_map->vma_list;

	while (vma != NULL) {
		for (uint64_t vaddr = (~0xfff) & vma->vma_start; vaddr < vma->vma_end;
				vaddr += 4096) {
//			printf(" trying to cleanup process page %p \n", vaddr);
			free_process_page_for_process_virt_addr(task, pml_virt, vaddr,
					pv_map_node);
		}
		vma = vma->vma_next;
	}
}

void free_ptables(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t* pml_virt) {
	pv_map_t* pt_pv_map = init_pv_map();
	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t ptable = vaddr_of_ptable(p->virtual_addr);
//		printf(" trying to free ptable %p \n", ptable);
		free_process_page_for_process_virt_addr(task, pml_virt, ptable,
				pt_pv_map);
	}
	free_pv_map(pt_pv_map);
}

void free_pdirs(task_struct_t * task, pv_map_t* pv_map_node, uint64_t* pml_virt) {
	pv_map_t* pdir_pv_map = init_pv_map();

	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t pdir = vaddr_of_pdir(p->virtual_addr);
//		printf(" trying to free pdir %p \n", pdir);
		free_process_page_for_process_virt_addr(task, pml_virt, pdir,
				pdir_pv_map);
	}

	free_pv_map(pdir_pv_map);
}

void free_pdir_ptrs(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t* pml_virt) {
	pv_map_t* pdir_ptr_pv_map = init_pv_map();

	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t pdir_ptr_base = vaddr_of_pdir_ptr(p->virtual_addr);
//		printf(" trying to free pdir_ptr_base %p \n", pdir_ptr_base);
		free_process_page_for_process_virt_addr(task, pml_virt, pdir_ptr_base,
				pdir_ptr_pv_map);
	}

	free_pv_map(pdir_ptr_pv_map);
}

void cleanup_ptables(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t *pml_virt) {
	// pv_map_node list has all virtual and phys addrs of all pages
	// present for the process we are trying to free and not that
	// of the current process. Use this to obtain their page table
	// pages and free them.
	pv_map_t* pml_pv_map = init_pv_map();

	uint64_t pml4_base_vaddr = 0xFFFFFF7FBFDFEFF0;

	free_ptables(task, pv_map_node, pml_virt);

	free_pdirs(task, pv_map_node, pml_virt);

	free_pdir_ptrs(task, pv_map_node, pml_virt);

//	printf(" trying to free pml %p \n", pml4_base_vaddr);
	// free pml base
	free_process_page_for_process_virt_addr(task, pml_virt, pml4_base_vaddr,
			pml_pv_map);

	free_pv_map(pml_pv_map);
}

void cleanup_kernel_stack(volatile task_struct_t * task) {
	if (task->kernel_stk_kmalloc_addr != 0) {
		//printf(" kernel stack freeing %p ", task->kernel_stk_kmalloc_addr);

		kfree((uint64_t *) task->kernel_stk_kmalloc_addr);
	}
}

void cleanup_both_stk_kernel_process(task_struct_t *task) {
	if (task->kernel_stk_kmalloc_addr != 0) {
		kfree((uint64_t *) task->kernel_stk_kmalloc_addr);
	}
	if (task->user_stk_kmalloc_addr != 0) {
		kfree((uint64_t *) task->user_stk_kmalloc_addr);
	}
}

void kfree_tstruct(task_struct_t* task) {
	//cleanup_fds(task);
	task->pid = 0;
	task->ppid = 0;
	task->state.rsp = 0;
	task->state.rip = 0;

	//page table stuff
	task->state.cr3 = 0;
	//flags
	task->state.flags = 0;
	task->state.kernel_rsp = 0;
	task->next = NULL;
	task->prev = NULL;
	task->mem_map = 0;
	for (uint64_t i = 0; i < 100; i++)
		task->executable[i] = '\0';
	task->p_state = 0;
	for (uint64_t i = 0; i < MAX_NUMBER_FILES; i++)
		task->filearray[i] = NULL;
	task->waiting_for = 0;
	task->is_kernel_process = 0;
	for (uint64_t i = 0; i < 100; i++)
		task->pwd[i] = '\0';
	task->is_background = 0;
	task->kernel_stk_kmalloc_addr = 0;
	task->user_stk_kmalloc_addr = 0;

	kfree(task);
}

void cleanup_process(task_struct_t * task) {
	if (task->is_kernel_process == 0) {
		// cleanup the mem_map, vmas, file descriptors,
		// page tables and then the task_struct itself

		uint64_t * pml_virt = get_virtual_location(0);
		setup_page_tables_after_cr3_update((uint64_t) pml_virt, task->state.cr3,
				1, 1, 0);

		pv_map_t* pv_map_node = init_pv_map();

		cleanup_process_pages(task, pv_map_node, pml_virt);

		cleanup_mem_map(task);

		cleanup_kernel_stack(task);

		cleanup_ptables(task, pv_map_node, pml_virt);

		free_pv_map(pv_map_node);
		//--> infinite loop of get free pages called if this is enabled

	} else {
		cleanup_both_stk_kernel_process(task);
	}
	kfree_tstruct(task);
//	space_msg();

	zero_dirty_free_pages(10);
//	space_msg();

}
