//#include <sys/paging.h>
//#include <sys/freelist.h>
//#include <sys/kmalloc.h>
//
//void free_vma(vma_t* q) {
//	if (q != NULL) {
//		kfree(q);
//	}
//}
//
//void cleanup_vmas(vma_t * vma) {
//	vma_t * p = vma;
//	vma_t * q = NULL;
//	while (p != NULL) {
//		free_vma(q);
//		q = p;
//		p = p->next;
//	}
//	free_vma(q);
//}
//
//void cleanup_mem_map(task_struct_t * task) {
//	mem_desc_t * mem_map = task->mem_map;
//	cleanup_vmas(mem_map->vma_list);
//	cleanup_vmas(mem_map->vma_cache);
//	kfree(mem_map);
//}
//
//void cleanup_fds(task_struct_t * task) {
//	// todo: this. also, need to keep the ptr for beginning of buffer
//
//}
//
//void free_page(uint64_t * phys_addr) {
//	decrease_ref_count(phys_addr);
//}
//
//void cleanup_process_pages(task_struct_t * task) {
//	mem_desc_t * mem_map = task->mem_map;
//	vma_t * vma = mem_map->vma_list;
//	while (vma != NULL) {
//
//	}
//}
//
//void cleanup_ptables(task_struct_t * task) {
//
//}
//
//void cleanup_kernel_stack(task_struct_t * task) {
//	uint64_t kernel_stack_base = task->state.kernel_rsp & (~0xfff);
//	kfree(kernel_stack_base);
//}
//void cleanup_process(task_struct_t * task) {
//	// cleanup the mem_map, vmas, file descriptors,
//	// page tables and then the task_struct itself
//	cleanup_process_pages(task);
//	cleanup_mem_map(task);
//	cleanup_fds(task);
//	cleanup_kernel_stack(task);
//	cleanup_ptables(task);
//}
