#include<sys/system_calls.h>
#include<sys/sbunix.h>
#include<sys/system_calls.h>
#include<sys/fork.h>
#include<sys/process.h>
int write_system_call(int fd, const void *buff, size_t count){
	if(fd != 1){//EACCES error
		return -13;
	}
//	if(count>PAGE_SIZE){//cannot write more than a page at a time
//		return -27;
//	}
	char *print_buffer = (char *) buff;
	int printed = 0;
	while (printed < count && print_buffer[printed] != '\0') {
		char character = print_buffer[printed];
		write_char_to_vid_mem(character, 0);
		printed++;
	}
	return printed;
}

int fork_sys_call() {
	return do_fork();
}
uint64_t brk_system_call(uint64_t value){
	mem_desc_t * mem_ptr = currenttask->mem_map;
	uint64_t current_brk = mem_ptr->brk;
	if(value == 0){
		return current_brk;
	}
	else{
		//ASSUMPTION: THERE IS NO OTHER ALLOCATION BETWEEN HEAP AND STACK
		//basically no random anon regions (eg: mmap)
		//stack:4 //
		uint64_t new_brk = value;
//		vma_t * temp_vma = mem_ptr->vma_list;
		vma_t *stack_vma = NULL;
		vma_t *heap_vma = NULL;
		for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL;
				temp_vma = temp_vma->vma_next){
			if(temp_vma->type == 5){
				heap_vma = temp_vma;
			}
			if(temp_vma->type == 4){
				stack_vma = temp_vma;
			}
		}							//not sure what is the stack top vma_start or end so marking both
		if(!(new_brk>heap_vma->vma_end && new_brk < stack_vma->vma_start && new_brk < stack_vma->vma_end)){
			return current_brk;
		}
		if(heap_vma !=NULL){
			heap_vma->vma_end = new_brk;
			mem_ptr->brk = new_brk;
			return new_brk;
		}

	}
	return current_brk;
}
