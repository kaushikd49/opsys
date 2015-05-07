#include<sys/system_calls.h>
#include<sys/sbunix.h>
#include<sys/system_calls.h>
#include<sys/fork.h>
#include<sys/process.h>
#include<errno.h>
#include<sys/tarfs_FS.h>
#include<sys/tarfs.h>
uint64_t strlen(const char *str) {
	uint64_t current = 0;
	uint64_t i = 0;
	while (str[i] != '\0') {
		i++;
		current++;
	}
	return current;
}
int strcmp_lazy(char *string1, char *string2) {
	size_t len = 0;
	while (string1[len] != '\0' && string2[len] != '\0') {
		if (string1[len] != string2[len])
			break;
		len++;
	}
	if (string1[len] == string2[len])
		return 0;
	else if (string1[len] == '\0'
			|| (string2[len] != '\0' && string1[len] < string2[len])) {
		return (int) string2[len];
	} else {
		return (int) string1[len];
	}

}



int kill_from_queue(pid_t pid, task_struct_t *queue) {
	int visited = 0;
	for (task_struct_t *t = queue; t != queue || visited == 0;
			t = t->next) {
		visited = 1;
		if (t->pid == pid) {
			if(t->is_kernel_process)
				return -2;

			mark_as_terminated(t);
			return 0;
		}
		// todo: return value conventions
	}
	return 999;
}

int kill_system_call(pid_t pid) {
	int rQRes = kill_from_queue(pid, currenttask); 
	int wQRes = kill_from_queue(pid, waitingtask); 
	
	if(rQRes <= 0) {
		return rQRes;
	} else if(wQRes <= 0){
		return wQRes;
	} else {
		return -1;
	}
}

void print_state(char state[], task_struct_t *t) {
	printf("|  %d  |  %d   | %s |   %d   |\n", t->pid, t->ppid, state,
			t->is_kernel_process);
}

void print_ps(task_struct_t * task) {
	int visited = 0;
	for (task_struct_t* t = task; t != NULL && (t != task || visited == 0); t =
			t->next) {
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
}

int ps_system_call() {

	printf("----------PROCESS STATE----------\n");
	printf("---------------------------------\n");
	printf("| pid | ppid |  state  | iskrnl |\n");
	printf("---------------------------------\n");
	print_ps(currenttask);
	print_ps(waitingtask);
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

//int write_system_call(int fd, const void *buff, size_t count){
//
//	if(currenttask->filearray[fd] == stdout_fd){
//		char *print_buffer = (char *) buff;
//		int printed = 0;
//		while (printed < count && print_buffer[printed] != '\0') {
//			char character = print_buffer[printed];
//			write_char_to_vid_mem(character, 0);
//			printed++;
//		}
//		return printed;
//	}
//	else{
//		//still need to test and add error cases
//		//havnt decided how to assign pages to the buffer
//		printf("other half of write");
//		char *copy_buf = (char *)buff;
//		int printed = 0;
//		while(printed < count && copy_buf[printed] != '\0'){
//			if(need_wait(fd)){
//				currenttask->filearray[fd]->ready = 0;
//				currenttask->p_status = TASK_WAITING;
//				change_read_end_pipe_status(fd, 1);
//				volatile int check = 0;
//				while(check == 0){
//					check = currenttask->filearray[fd]->ready;
//				}
//			}
//			char character  = copy_buf[printed];
//			char *copy_to = (char *)currenttask->filearray[fd]->current_pointer;
//			*copy_to = character;
//			copy_to++;
//			printed++;
//		}
//		return printed++;
//	}
//	return -1;
//}

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

int pipe_system_call(int pipe[2]) {
	return pipe_tarfs(pipe);
}

int pwd_system_call(char *buffer, uint64_t size){
	if((buffer == NULL) ||(size == 0 && buffer != NULL)){
		return -EINVAL;
	}
	if(strlen(currenttask->pwd) > size){
		return -ERANGE;
	}
	strcpy(buffer, currenttask->pwd);
	return 0;
}
int cd_errno = 0;
void clear_this_segment(char *this_segment){
	for(uint64_t i = 0; i < 100; i++){
		this_segment[i] = '\0';
	}
}
int remove_prev_segment(char *prefix,int prefix_index){
	if(prefix_index == 0){
		return -1;
	}
	else{
		prefix_index--;
		prefix[prefix_index] = '\0';
		while(prefix[prefix_index] != '/' && prefix_index>=0){
			prefix[prefix_index--] = '\0';
		}

	}
	prefix_index++;
	return prefix_index;
}
int add_to_prefix(char *prefix, char *this_segment, int p_index){
	int i = 0;
	while(p_index < 100 && this_segment[i] != '\0'){
		prefix[p_index] = this_segment[i];
		p_index++;
		i++;
	}
	if(this_segment[i] == '\0'){
		return p_index;
	}
	else if(p_index == 100){
		cd_errno = -ENAMETOOLONG;
	}
	return p_index;
}

char *expand_cd_buffer(char *buffer, int len, char *prefix){
	char this_segment[100];
	int j = 0;
	int prefix_index = 0;
	clear_this_segment(prefix);
	clear_this_segment(this_segment);
	if(buffer[0] == '/'){
		buffer = buffer + 1;
		len = len -1;
	}
	else{
		strcpy(prefix, currenttask->pwd);
		prefix_index = strlen(prefix);
	}

	for(uint64_t i = 0; i < len; i++){
		if(buffer[i] == '/'){
			if(strcmp_lazy(this_segment, "..") == 0){
				prefix_index = remove_prev_segment(prefix, prefix_index);
				if(prefix_index == -1){
					cd_errno = -EFAULT;
					return NULL;
				}
			}
			else if(strcmp_lazy(this_segment, ".") != 0){
				prefix_index = add_to_prefix(prefix, this_segment,  prefix_index);
				if(prefix_index == -1){
					return NULL;
				}
				prefix[prefix_index] = '/';
				prefix_index++;
			}
			j = 0;
			clear_this_segment(this_segment);
		}
		else{
			this_segment[j++] = buffer[i];
		}
	}
	if(this_segment[0] != '\0'){
		strcpy(&prefix[prefix_index], this_segment);
		prefix_index = prefix_index + j;
		prefix[prefix_index] = '/';
		prefix_index++;
		prefix[prefix_index] = '\0';
	}
	else{
		prefix[prefix_index] = '\0';
	}
	return prefix;
}
int is_valid_directory(char *file_name){
	struct posix_header_ustar *current =
				(struct posix_header_ustar *) &_binary_tarfs_start;
		int i =0;
		while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
			if(strcmp_lazy(file_name, current->name) == 0){
				if(current->typeflag[0] != '5'){
					cd_errno = -ENOTDIR;
					return 0;
				}
				printf("%s %s: found file", current->name, current->magic);
				return 1;
			}
	//			if(i %5 == 0)
	//				printf("\n");

				//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
			uint64_t header_next = (uint64_t) ((align(
					convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))
					+ sizeof(struct posix_header_ustar) + (uint64_t) current);
			//		printf("header : %x", header_next);
			current = (struct posix_header_ustar *) (header_next);
			i++;
		}
		cd_errno = -ENOENT;
		return 0;
}
task_struct_t *find_parent(int pid){
	task_struct_t *current = waitingtask;
	if(current !=NULL){
		do{
			if(current->pid == pid){
				return current;
			}
			current = current->next;
		}while(current!=waitingtask);

	}
	current = currenttask;
	if(currenttask != NULL){
		do{
			if(current->pid == pid){
				return current;
			}
			current = current->next;
		}while(current!=currenttask);
	}
	return NULL;
}
int cd_system_call(char *buffer){
	int len = strlen(buffer);
	if(len > 100){
		return -ENAMETOOLONG;
	}
	char *answer = kmalloc(100);
	answer = expand_cd_buffer(buffer, len, answer);
	if(answer == NULL){
		kfree(answer);
		return cd_errno;
	}
	if(is_valid_directory(answer)){
		task_struct_t *parent_process = find_parent(currenttask->ppid);
		if(parent_process == NULL){
			kfree(answer);
			return -EIO;
		}
		printf("answer: %s pwd: %s %d %d %d\n", answer, parent_process->pwd, parent_process->pid, currenttask->pid, currenttask->ppid);
		strcpy(currenttask->pwd, answer);
		printf("answer: %s pwd: %s\n", answer, parent_process->pwd);
	}
	else{
		kfree(answer);
		return cd_errno;
	}
	kfree(answer);
	return 0;
}
