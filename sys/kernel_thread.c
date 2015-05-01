#include<sys/process.h>
#include<sys/scheduling.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include<sys/isr_stuff.h>
#include <sys/kmalloc.h>
#include<sys/tarfs_FS.h>
#include<sys/kernel_thread.h>
#define mask(addr, mask) ((addr | (mask))+1)
struct read_blocked{
	int pid;
	int read_fd;
	int write_fd;
	int read_write; // which end of the pipe is blocked
	struct read_blocked *next;
};
typedef struct read_blocked read_blocked_t;
read_blocked_t *read_blocked_list = NULL;
int need_wait(int fd){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for(uint64_t i = 0; i < MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_RDONLY) != 0){
			if(((uint64_t)(currenttask->filearray[i]->current_pointer)& ~(0xfff)) == c_pointer){
				if(((uint64_t)(currenttask->filearray[fd]->current_pointer)+1)%mask((uint64_t)(currenttask->filearray[fd]->current_pointer), 0xfff) == (uint64_t)(currenttask->filearray[i]->current_pointer)){
					return 1;
				}
			}
		}
	}
	return 0;
}

int no_read_end(int fd){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for(uint64_t i = 0; i < MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_RDONLY) != 0){
			if(((uint64_t)(currenttask->filearray[i]->current_pointer)& ~(0xfff)) == c_pointer){
					return 0;
			}
		}
	}
	return 1;
}
void *find_write_end(int fd){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for(uint64_t i = 0; i < MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_RDONLY) != 0){
			if(((uint64_t)(currenttask->filearray[i]->current_pointer)& ~(0xfff)) == c_pointer){
					return currenttask->filearray[i]->current_pointer;
			}
		}
	}
	return NULL;
}
void add_read_blocked_list(read_blocked_t *node){
	if(read_blocked_list == NULL){
		read_blocked_list = node;
		return;
	}
	node->next = read_blocked_list;
	read_blocked_list = node;
}
read_blocked_t *remove_read_blocked_list(read_blocked_t *current){
	read_blocked_t *prev = read_blocked_list;
	if(prev == current){
		read_blocked_list = current->next;
		kfree(current);
		return read_blocked_list;
	}
	else{
		while(prev->next != current){
			prev = prev->next;
		}
		prev->next = current->next;
		kfree(current);
		return prev->next;
	}
	return NULL;
}
//void test_main(){
//	uint64_t i = 1;
//	printf("inside kernel thread %d", i);
//	limit = limit << 2;
//	while(i < limit){
//		//if(i%100000000 == 0){
//		//	printf("%d ", i);
//		//	i++;
//		//}
//		i++;
//	}
//	quit_kernel_thread();
//}
void noop(int i){

}
inline void fake_preempt(int flag){
	__asm__ __volatile__(
			"int $99"
			:
			:"S"(flag));

}
void waiting_to_running_q(){

	while(1){
		__asm__ __volatile__("cli");
		volatile task_struct_t *current_waiting = waitingtask;
		do{

			if(current_waiting != NULL && current_waiting->p_state == STATE_READY){

//				printf("\n moving back bitchas");
				move_process_waitq_to_runq(current_waiting->pid);

				current_waiting = waitingtask;

			}
			if(current_waiting !=NULL)
				current_waiting = current_waiting->next;
		}while(waitingtask !=NULL && current_waiting != waitingtask);
		__asm__ __volatile__("sti");
		fake_preempt(1);
	}
}
int check_ready(file_desc_t *current_fd){
	return current_fd->ready;
}
int check_busy(file_desc_t *current_fd){
	return current_fd->busy;
}
int empty_buffer(void *read_end, void *write_end){
	if((uint64_t)read_end > (uint64_t)write_end){
		return 1;
	}
	return 0;
}

int write_end_pipe_status(int fd, int status){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	uint64_t i = 0;
	int check = 0;
	for(i = 0; i < MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_WRONLY) != 0){
			if(mask((uint64_t)(currenttask->filearray[i]->current_pointer), 0xfff) == c_pointer){
				currenttask->filearray[i]->ready = 1;
				check = 1;
				break;
			}
		}
	}
	if(check == 0){
		return -1;
	}
	read_blocked_t *new_node = kmalloc(sizeof(read_blocked_t));
	new_node->pid = currenttask->pid;
	new_node->read_fd = fd;
	new_node->write_fd = i;
	new_node->read_write = 2; // read end is going to be blocked
	new_node->next = NULL;
	add_read_blocked_list(new_node);
	fake_preempt(2);
	return 1;
	__asm__ __volatile__("cli");
}
void read_thread_process(int fd, char *buffer, uint64_t size){
	file_desc_t *current_fd = currenttask->filearray[fd];
	while(check_busy(current_fd) == 1 && current_fd->current_process !=currenttask->pid);
	__asm__ __volatile__("cli");
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
	__asm__ __volatile__("sti");
	while(check_ready(current_fd) == 0);
	__asm__ __volatile__("cli");
	char *current_pointer = currenttask->filearray[fd]->current_pointer;
	int copied = 0;
	if((int)(currenttask->filearray[fd]->size)!=-999){
		char *end_file = current_pointer + currenttask->filearray[fd]->size;
		char *buff = (char *)buffer;
		while(copied < size && current_pointer < end_file ){
			buff[copied] = *current_pointer;
			current_pointer++;
			currenttask->filearray[fd]->current_pointer++;
			copied++;
		}
	}
	else{
		char *read_end = current_fd->current_pointer;
		char *write_end = find_write_end(fd);
		if(write_end != NULL){
			if(empty_buffer(read_end, write_end)){
				volatile int check = 0;
				while(check == 0){
					currenttask->filearray[fd]->ready = 0;
					currenttask->p_state = STATE_WAITING;
					int res = write_end_pipe_status(fd, 1);
					if(res == -1)
						break;
					check = currenttask->filearray[fd]->ready;
				}
			}
			while(copied < size){
				char character  = *read_end;
				buffer[copied] = character;
				uint64_t mod_val = mask((uint64_t)read_end, 0xfff);
				read_end++;
				read_end = (char *)((uint64_t)read_end%mod_val);
				current_fd->current_pointer = read_end;
				copied++;
				if(character == '\0'){
					break;
				}
				if(empty_buffer(read_end, write_end)){
					volatile int check = 0;
					while(check == 0){
						currenttask->filearray[fd]->ready = 0;
						currenttask->p_state = STATE_WAITING;
						int res = write_end_pipe_status(fd, 1);
						if(res == -1)
							break;
						check = currenttask->filearray[fd]->ready;
					}
				}


			}
			if(read_end == write_end+1){
				current_fd->current_pointer = write_end;
			}
			uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
			uint64_t i = 0;
			for(i = 0; i < MAX_FILES_SYSTEM; i++){
				if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_WRONLY) != 0){
					if(((uint64_t)(currenttask->filearray[i]->current_pointer)& ~(0xfff)) == c_pointer){
						currenttask->filearray[i]->ready = 1;
						break;
					}
				}
			}
		}
	}
	int ppid = currenttask->ppid;
	task_struct_t *current_waiting = waitingtask;
	do{
		if(current_waiting!=NULL){
			if(current_waiting->pid == ppid){
				regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
				regs->rax = copied;
				current_waiting->p_state = STATE_READY;
				break;
			}
		}
		if(current_waiting!=NULL)
			current_waiting = current_waiting->next;
	}while(current_waiting!=waitingtask);
	if(fd == 0)
		clear_keyboard_busy();
	current_fd->ready = 0;
	current_fd->busy = 0;
	quit_kernel_thread();
	//	return copied;

}

int read_end_pipe_status(int fd, int status){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	uint64_t i = 0;
	int check = 0;
	for(i = 0; i < MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_RDONLY) != 0){
			if(((uint64_t)(currenttask->filearray[i]->current_pointer)& ~(0xfff)) == c_pointer){
				currenttask->filearray[i]->ready = 1;
				check = 1;
				break;
			}
		}
	}
	if(check == 0){
		return -1;
	}
	read_blocked_t *new_node = kmalloc(sizeof(read_blocked_t));
	new_node->pid = currenttask->pid;
	new_node->read_fd = i;
	new_node->write_fd = fd;
	new_node->read_write = 1; // 1 is write end is blocked
	new_node->next = NULL;
	add_read_blocked_list(new_node);
	fake_preempt(2);
	__asm__ __volatile__("cli");
	return 1;
}

void write_thread_process(int fd, void *buffer, uint64_t size){
	file_desc_t *current_fd = currenttask->filearray[fd];
	while(check_busy(current_fd) == 1 && current_fd->current_process != currenttask->pid);
	__asm__ __volatile__("cli");
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
	int printed = 0;
	if(currenttask->filearray[fd] == stdout_fd){
		char *print_buffer = (char *) buffer;
		while (printed < size && print_buffer[printed] != '\0') {
			char character = print_buffer[printed];
			write_char_to_vid_mem(character, 0);
			printed++;
		}
	}else{
		char *copy_buf = (char *)buffer;

		while(printed < size && copy_buf[printed] != '\0'){
			if(no_read_end(fd)){
				break;
			}
			char character  = copy_buf[printed];
			char *copy_to = (char *)currenttask->filearray[fd]->current_pointer;
			*copy_to = character;
			printed++;
			if(need_wait(fd)){
				volatile int check = 0;
				while(check == 0){
					currenttask->filearray[fd]->ready = 0;
					currenttask->p_state = STATE_WAITING;
					int res = read_end_pipe_status(fd, 1);
					if(res == -1)
						break;
					check = currenttask->filearray[fd]->ready;
				}
			}
			uint64_t mod_val = mask((uint64_t)copy_to, 0xfff);
			copy_to++;
			copy_to = (char *)((uint64_t)copy_to%mod_val);
			currenttask->filearray[fd]->current_pointer = copy_to;
		}
		if(copy_buf[printed] == '\0'){
			char *copy_to = (char *)currenttask->filearray[fd]->current_pointer;
			*copy_to = '\0';
//			uint64_t mod_val = mask((uint64_t)copy_to, 0xfff);
//			copy_to++;
//			copy_to = (uint64_t)copy_to%mod_val;
//			currenttask->filearray[fd]->current_pointer = copy_to;
		}
		uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
		uint64_t i = 0;
		for(i = 0; i < MAX_FILES_SYSTEM; i++){
			if(currenttask->filearray[i] != NULL && i != fd && (currenttask->filearray[i]->flags &O_RDONLY) != 0){
				if(((uint64_t)(currenttask->filearray[i]->current_pointer) & (~0xfff)) == c_pointer){
					currenttask->filearray[i]->ready = 1;
					break;
				}
			}
		}
	}
	int ppid = currenttask->ppid;
	task_struct_t *current_waiting = waitingtask;
	do{
		if(current_waiting!=NULL){
			if(current_waiting->pid == ppid){
				regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
				regs->rax = printed;
				current_waiting->p_state = STATE_READY;
				break;
			}
		}
		if(current_waiting!=NULL)
			current_waiting = current_waiting->next;
	}while(current_waiting!=waitingtask);
	if(currenttask->filearray[fd] == stdout_fd)
		clear_keyboard_busy();
	current_fd->ready = 0;
	current_fd->busy = 0;
	quit_kernel_thread();
}
//clears keyoard buffer when a new process begins writing
void clear_keyboard_busy(){
	char *current = (char *)(input_buffer);
	for(uint64_t i = 0; i < 0x1000 ; i++){
		*current = 0;
		current++;
	}
	currenttask->filearray[0]->current_pointer = (char *)input_buffer;

}

void find_child(task_struct_t *temp){

	int ppid = temp->pid;
	int wait_for = temp->waiting_for;
	if(wait_for == -1){
		task_struct_t *temp2 = waitingtask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->ppid == ppid){
				return;
			}
			temp2 = temp2->next;
		}while(temp2!=waitingtask);
		temp2 = currenttask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->ppid == ppid){
				return;
			}
			temp2 = temp2->next;
		}while(temp2!=currenttask);
	}
	else{
		task_struct_t *temp2 = waitingtask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->pid == wait_for){
				return;
			}
			temp2 = temp2->next;
		}while(temp2!=waitingtask);
		temp2 = currenttask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->pid == wait_for){
				return;
			}
			temp2 = temp2->next;
		}while(temp2!=currenttask);
	}
//	printf("daemon activated");
	temp->p_state = STATE_READY;
}

void check_user_process_waitpid_daemon(){


	while(1){
		__asm__ __volatile__("cli");
		task_struct_t *temp = waitingtask;

		do{

			if(temp!=NULL && temp->is_kernel_process == 0 && temp->waiting_for != 999){
				find_child(temp);
			}
			if(temp!=NULL)
				temp = temp->next;
		}while(temp!=waitingtask);
		__asm__ __volatile__("sti");
		//	for(int i = 0; i<1000000; i++){
		//		noop(i);
		//	}
		fake_preempt(1);
	}
}
task_struct_t *check_unblock_waitq(int pid){
	task_struct_t *temp = waitingtask;
	do{
		if(temp!=NULL && temp->pid == pid){
			return temp;
		}
		if(temp!=NULL)
			temp = temp->next;
	}while(temp!=waitingtask);
	return NULL;
}
void return_blocking_rw_to_runq(){
	while(1){
		__asm__ __volatile__("cli");
		read_blocked_t *current = read_blocked_list;
		if(current != NULL){
			do{
				task_struct_t *temp = NULL;
				if(current!=NULL){
					int pid = current->pid;
					temp = check_unblock_waitq(pid);}
				if(temp!=NULL && temp->p_state == STATE_WAITING){
					if(current->read_write == 1){
						int fd = current->write_fd;
						if(temp->filearray[fd]->ready == 1){
							temp->p_state = STATE_READY;
							current = remove_read_blocked_list(current);
							continue;
						}
					}
					else{
						int fd = current->read_fd;
						if(temp->filearray[fd]->ready == 1){
							temp->p_state = STATE_READY;
							current = remove_read_blocked_list(current);
							continue;
						}
					}
				}
				if(current !=NULL)
					current = current->next;
			}while(current!= NULL);
		}
		__asm__ __volatile__("sti");
		fake_preempt(1);
	}
}
//void clean_up_processes(){
//	if(waitingtask == NULL){
//		return NULL;
//	}
//	task_struct *current = waitingtask;
//	int count = 0;
//	do{
//		count++;
//		current = current->next;
//	}while(current!=waitingtask)
//	task_struct_t *prev = NULL;
//	task_struct_t *current = waitingtask;
//	while(current!=NULL &&current){
//
//	}
//}
