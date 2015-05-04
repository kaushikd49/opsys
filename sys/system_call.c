#include<sys/system_calls.h>
#include<sys/sbunix.h>
#include<sys/system_calls.h>
#include<sys/fork.h>
#include<sys/process.h>

int kill_system_call(pid_t pid) {
	int visited = 0;
	for (task_struct_t *t = currenttask; t != currenttask || visited == 0;
			t = t->next) {
		visited = 1;
		if (t->pid == pid) {
			mark_as_terminated(t);
			return 0;
		}
		// todo: return value conventions
	}
	return -1;
}
void print_state(char state[], task_struct_t *t) {
	printf("|  %d  |  %d   | %s |   %d   |\n", t->pid, t->ppid, state,
			t->is_kernel_process);
}

int ps_system_call() {
	int visited = 0;
	printf("----------PROCESS STATE----------\n");
	printf("---------------------------------\n");
	printf("| pid | ppid |  state  | iskrnl |\n");
	printf("---------------------------------\n");
	for (task_struct_t *t = currenttask; t != currenttask || visited == 0;
			t = t->next) {
		visited = 1;
		if (t->p_state == STATE_RUNNING) {
			print_state("RUNNING", t);
		} else if (t->p_state == STATE_WAITING) {
			print_state("WAITING", t);
		} else if (t->p_state == STATE_READY) {
			print_state("READY", t);
		} else if (t->p_state == STATE_TERMINATED) {
			print_state("TERMINATED", t);
		}
	}
	printf("---------------------------------\n");
	return 0;
}

int write_system_call(int fd, const void *buff, size_t count) {

	if (currenttask->filearray[fd] == stdout_fd) {
		char *print_buffer = (char *) buff;
		int printed = 0;
		while (printed < count && print_buffer[printed] != '\0') {
			char character = print_buffer[printed];
			write_char_to_vid_mem(character, 0);
			printed++;
		}
		return printed;
	} else {
		//still need to test and add error cases
		//havnt decided how to assign pages to the buffer
		printf("other half of write");
		char *copy_buf = (char *) buff;
		int printed = 0;
		while (printed < count && copy_buf[printed] != '\0') {
			char character = copy_buf[printed];
			char *copy_to = (char *) currenttask->filearray[fd]->current_pointer;
			*copy_to = character;
			copy_to++;
			printed++;
		}
		return printed++;
	}
	return -1;
}

int fork_sys_call(uint64_t stack_top) {
	return do_fork(stack_top);
}
uint64_t brk_system_call(uint64_t value) {
	mem_desc_t * mem_ptr = currenttask->mem_map;
	uint64_t current_brk = mem_ptr->brk;
	if (value == 0) {
		return current_brk;
	} else {
		//ASSUMPTION: THERE IS NO OTHER ALLOCATION BETWEEN HEAP AND STACK
		//basically no random anon regions (eg: mmap)
		//stack:4 //
		uint64_t new_brk = value;
//		vma_t * temp_vma = mem_ptr->vma_list;
		vma_t *stack_vma = NULL;
		vma_t *heap_vma = NULL;
		for (vma_t *temp_vma = mem_ptr->vma_list; temp_vma != NULL; temp_vma =
				temp_vma->vma_next) {
			if (temp_vma->type == 5) {
				heap_vma = temp_vma;
			}
			if (temp_vma->type == 4) {
				stack_vma = temp_vma;
			}
		}	//not sure what is the stack top vma_start or end so marking both
		if (!(new_brk > heap_vma->vma_end && new_brk < stack_vma->vma_start
				&& new_brk < stack_vma->vma_end)) {
			printf("cb: %x", current_brk);
			return current_brk;
		}
		if (heap_vma != NULL) {
			heap_vma->vma_end = new_brk;
			mem_ptr->brk = new_brk;
			printf("nb: %x", current_brk);
			return new_brk;
		}

	}
	return current_brk;
}

uint64_t wait_pid(int pid, int *status, int options, uint64_t stack_top) {
	return temp_preempt_waitpid(pid, status, options, stack_top);
}

uint64_t nanosleep_sys_call(const struct timespec *rqtp, struct timespec *rmtp,
		uint64_t stack_top) {
	return temp_preempt_nanosleep(rqtp, rmtp, stack_top);
}
uint64_t execve_sys_call(char *binary, char **argv, char **envp,
		uint64_t stack_top) {
	return execve_process(binary, argv, envp, stack_top);
}
