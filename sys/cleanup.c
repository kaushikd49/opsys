#include <sys/paging.h>
#include <sys/freelist.h>
#include <sys/kmalloc.h>
#include <sys/utils.h>
#include <sys/process.h>
#include <sys/sbunix.h>

void free_vma(vma_t* q) {
	if (q != NULL) {
//		kfree(q);
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

void cleanup_mem_map(task_struct_t * task) {
	mem_desc_t * mem_map = task->mem_map;
	cleanup_vmas(mem_map->vma_list);
	cleanup_vmas(mem_map->vma_cache);
//	kfree(mem_map);
}

void cleanup_fds(task_struct_t * task) {
	// todo: this. also, need to keep the ptr for beginning of buffer
}

void free_frame(uint64_t phys_addr) {
	printf(" freeing frame %p \n", phys_addr);
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

	uint64_t phys_addr = (uint64_t) next_entity_base((uint64_t *)*deepest_entity);

	if (res != 0) {
		return 0;
	} else {
		return phys_addr;
	}

}

void free_if_not_freed(pv_map_t* pv_map_node, uint64_t vaddr,
		uint64_t page_phys_addr) {
	if (if_not_contains_phys_addr(pv_map_node, vaddr, page_phys_addr)) {
		free_frame(page_phys_addr);
		cache_pv_mapping(pv_map_node, vaddr, page_phys_addr);
	}
}

void free_process_page_for_process_virt_addr(uint64_t* pml_virt, uint64_t vaddr,
		pv_map_t* pv_map_node) {
	// actual physical address of the page vaddr for the process with cr3 task->state.cr3
	uint64_t page_phys_addr = get_phys_from_virt_of_other_process(pml_virt,
			vaddr, pv_map_node);
	if (page_phys_addr != 0) {
		// physical page present in process
		free_if_not_freed(pv_map_node, vaddr, page_phys_addr);
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
			free_process_page_for_process_virt_addr(pml_virt, vaddr,
					pv_map_node);
		}
		vma = vma->vma_next;
	}
}

void free_ptables(pv_map_t* pv_map_node, uint64_t* pml_virt) {
	pv_map_t* pt_pv_map = init_pv_map();
	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t ptable = vaddr_of_ptable(p->virtual_addr);
		printf(" trying to free ptable %p \n", ptable);
		free_process_page_for_process_virt_addr(pml_virt, ptable, pt_pv_map);
	}
}

void free_pdirs(pv_map_t* pv_map_node, uint64_t* pml_virt) {
	pv_map_t* pdir_pv_map = init_pv_map();

	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t pdir = vaddr_of_pdir(p->virtual_addr);
		free_process_page_for_process_virt_addr(pml_virt, pdir, pdir_pv_map);
	}
}

void free_pdir_ptrs(pv_map_t* pv_map_node, uint64_t* pml_virt) {
	pv_map_t* pdir_ptr_pv_map = init_pv_map();

	for (pv_map_t* p = pv_map_node; p != NULL; p = p->next) {
		uint64_t pdir_ptr_base = vaddr_of_pdir_ptr(p->virtual_addr);
		free_process_page_for_process_virt_addr(pml_virt, pdir_ptr_base,
				pdir_ptr_pv_map);
	}
}

void cleanup_ptables(task_struct_t * task, pv_map_t* pv_map_node,
		uint64_t *pml_virt) {
	// pv_map_node list has all virtual and phys addrs of all pages
	// present for the process we are trying to free and not that
	// of the current process. Use this to obtain their page table
	// pages and free them.
	pv_map_t* pml_pv_map = init_pv_map();

	uint64_t pml4_base_vaddr = 0xFFFFFF7FBFDFEFF0;

	free_ptables(pv_map_node, pml_virt);

	free_pdirs(pv_map_node, pml_virt);

	free_pdir_ptrs(pv_map_node, pml_virt);

	// free pml base
	free_process_page_for_process_virt_addr(pml_virt, pml4_base_vaddr,
			pml_pv_map);
}

void cleanup_kernel_stack(task_struct_t * task) {
//	uint64_t kernel_stack_base = task->state.kernel_rsp & (~0xfff);
//	kfree((uint64_t *) kernel_stack_base);
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
		cleanup_fds(task);
		cleanup_kernel_stack(task);
		cleanup_ptables(task, pv_map_node, pml_virt);

//		free_pv_map(pv_map_node);
	}
}