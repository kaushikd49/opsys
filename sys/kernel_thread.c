#include<sys/process.h>
#include<sys/scheduling.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include<sys/isr_stuff.h>
#include <sys/kmalloc.h>
#include<sys/tarfs_FS.h>
#include<sys/kernel_thread.h>
#include<sys/cleanup.h>

extern uint64_t *global_keyboard;
extern pipe_struct_t *pipe_list;
#define mask(addr, maskq) ((addr | (maskq))+1)
struct read_blocked{
	int pid;
	pipe_struct_t *pipe_info;
	int read_write; // which end of the pipe is blocked
	struct read_blocked *next;
};
typedef struct read_blocked read_blocked_t;
read_blocked_t *read_blocked_list = NULL;
/////////////////////////////////////////////////////
pipe_struct_t *find_pipe_from_write(file_desc_t *fd_write){
	pipe_struct_t *current = pipe_list;
	if(current == NULL){
		return NULL;
	}
	while(current!=NULL){
		if(current->write_end == fd_write){
			return current;
		}
		current = current->next;
	}
	return NULL;
}
pipe_struct_t *find_pipe_from_read(file_desc_t *fd_read){
	pipe_struct_t *current = pipe_list;
	if(current == NULL){
		return NULL;
	}
	while(current!=NULL){
		if(current->read_end == fd_read){
			return current;
		}
		current = current->next;
	}
	return NULL;
}
///////////////////////////////////////////////////////

int need_wait(int fd){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for(uint64_t i = 0; i < MAX_FILES_OS; i++){
		file_desc_t *current = global_fd_array[i].fd;
		int count = global_fd_array[i].count;
		if(current != NULL&& (current->flags == 0)){
			if((((uint64_t)(current->current_pointer)& ~(0xfff)) == c_pointer)  && count>0){
				uint64_t next_addr = (uint64_t)(currenttask->filearray[fd]->current_pointer)+1;
				uint64_t mask_addr = mask((uint64_t)(currenttask->filearray[fd]->current_pointer), 0xfff);
				if(next_addr == mask_addr){
					next_addr = next_addr - 0x1000;
				}
				if(next_addr == (uint64_t)(current->current_pointer)){
					printf("need wait");
					return 1;

				}
			}
			else{

				return 0;
			}
		}
	}
	return -1;
}

int no_read_end(int fd) {
	uint64_t c_pointer =
			(uint64_t) (currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for (uint64_t i = 0; i < MAX_FILES_SYSTEM; i++) {
		file_desc_t *current = global_fd_array[i].fd;
		int count = global_fd_array[i].count;
		if (current != NULL && (current->flags & ~(2)) != 0) {
			if ((((uint64_t) (current->current_pointer) & ~(0xfff)) == c_pointer)
					&& count > 0) {
				return 0;
			}
		}
	}
	return 1;
}
void *find_write_end(int fd) {
	uint64_t c_pointer =
			(uint64_t) (currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		file_desc_t *current = global_fd_array[i].fd;
//		int count = global_fd_array[i].count;
		if(current != NULL && i != fd && current->flags == O_WRONLY){
			if(((uint64_t)(current->current_pointer)& ~(0xfff)) == c_pointer){
					return current->current_pointer;
			}
		}
	}
	return NULL;
}
void add_read_blocked_list(read_blocked_t *node) {
	if (read_blocked_list == NULL) {
		read_blocked_list = node;
		return;
	}
	node->next = read_blocked_list;
	read_blocked_list = node;
}
read_blocked_t *remove_read_blocked_list(volatile read_blocked_t *current){
	read_blocked_t *prev = read_blocked_list;
	if (prev == current) {
		read_blocked_list = current->next;
		current->next = NULL;
//		kfree(current);
		return read_blocked_list;
	} else {
		while (prev->next != current) {
			prev = prev->next;
		}
		prev->next = current->next;
//		kfree(current);
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
void noop(int i) {

}
inline void fake_preempt(int flag) {
	__asm__ __volatile__(
			"int $99"
			:
			:"S"(flag));

}

void waiting_to_running_q_body() {
	volatile task_struct_t* current_waiting = waitingtask;
	do {
		if (current_waiting != NULL
				&& current_waiting->p_state == STATE_READY) {
			//				printf("\n moving back bitchas");
			move_process_waitq_to_runq(current_waiting->pid);
			current_waiting = waitingtask;
		}
		if (current_waiting != NULL)
			current_waiting = current_waiting->next;
	} while (waitingtask != NULL && current_waiting != waitingtask);
}

void waiting_to_running_q(){

	while (1) {
		__asm__ __volatile__("cli");
		waiting_to_running_q_body();
		fake_preempt(1);
	}
}
int check_ready(file_desc_t *current_fd) {
	return current_fd->ready;
}
int check_busy(file_desc_t *current_fd){
	int ans = (current_fd->busy) & (current_fd->current_process != currenttask->pid);
	return ans;
}
//////////////////////////
void reserve_fd(file_desc_t* current_fd) {
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
}
int write_stdout(uint64_t size, void* buffer) {
	int printed = 0;
	char* print_buffer = (char*) buffer;
	while (printed < size && print_buffer[printed] != '\0') {
		char character = print_buffer[printed];
		write_char_to_vid_mem(character, 0);
		printed++;
	}
	write_char_to_vid_mem('\0', 0);
	return printed;
}
int get_pipe_size(pipe_struct_t* pipe_info) {
	return pipe_info->size;
}
void *get_next_address_circular(void *addr){
	uint64_t addr_uint = (uint64_t)addr;
	uint64_t addr_mod = addr_uint | 0xfff;
	if(addr_uint == addr_mod){
		addr_uint = addr_uint &~(0xfff);
	}
	else{
		addr_uint++;
	}
	return (void *)addr_uint;
}

void block_write_signal_read(pipe_struct_t* pipe_info) {
	pipe_info->write_end->ready = 0;
	currenttask->p_state = STATE_WAITING;
	pipe_info->read_end->ready = 1;
	read_blocked_t* new_node = kmalloc(sizeof(read_blocked_t));
	new_node->pid = currenttask->pid;
	new_node->pipe_info = pipe_info;
	new_node->read_write = 1;
	new_node->next = NULL;
	add_read_blocked_list(new_node);
	fake_preempt(2);
	__asm__ __volatile__("cli");
}

int is_read_end_dead(pipe_struct_t* pipe_info) {
	int c = 0;
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		volatile file_desc_t* current = global_fd_array[i].fd;
		volatile int count = global_fd_array[i].count;
		if (current == pipe_info->read_end && count <=1) {//todo:this value might change based on the decision we make on fd counts
			c = 1;
		}
	}
	return c;
}

int is_write_end_dead(pipe_struct_t* pipe_info) {
	int c = 0;
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		volatile file_desc_t* current = global_fd_array[i].fd;
		volatile int count = global_fd_array[i].count;
		if (current == pipe_info->write_end && count <= 1) {//todo:this value might change based on the decision we make on fd counts
			c = 1;
		}
	}
	return c;
}
int handle_pipe_write(uint64_t size, file_desc_t* current_fd, void* buffer) {
	pipe_struct_t* pipe_info = find_pipe_from_write(current_fd);
	char* copy_buf = (char*) buffer;
	int printed = 0;
	while (printed < size && copy_buf[printed] != '\0') {
		int sizep = get_pipe_size(pipe_info);
		if (sizep < 0x1000) {
			char character = copy_buf[printed];
//			printf("%c", character);
			char* copy_to = (char*) (pipe_info->write_end->current_pointer);
			*copy_to = character;
			pipe_info->size = pipe_info->size + 1;
			printed++;
			pipe_info->write_end->current_pointer = get_next_address_circular(
					pipe_info->write_end->current_pointer);
		} else {
			block_write_signal_read(pipe_info);
		}
		int read_status = is_read_end_dead(pipe_info);
		if(read_status == 1){
			break;
		}
	}
	pipe_info->read_end->ready = 1;
	return printed;
}
////////////////////////////

int empty_buffer(void *read_end, void *write_end){
	if((uint64_t)read_end == (uint64_t)write_end){
		return 1;
	}
	return 0;
}
int is_write_end_closed(int fd){
	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
	uint64_t i = 0;
	for(i = 0; i < MAX_FILES_OS; i++){
		file_desc_t *current = global_fd_array[i].fd;
		int count = global_fd_array[i].count;
		if(current != NULL && ((current->flags &O_WRONLY) != 0) && count > 0){
			if(((uint64_t)(current->current_pointer)& ~(0xfff)) == c_pointer){
				return 0;
			}
		}
//		else if{
//
//		}
	}
	return 1;
}

int handle_pipe_read(int copied, uint64_t size, file_desc_t* current_fd,
		char* buffer) {
	pipe_struct_t* pipe_info = find_pipe_from_read(current_fd);
	while (copied < size - 1) {
		int sizep = get_pipe_size(pipe_info);
		if (sizep > 0) {
			char character = *(pipe_info->read_end->current_pointer);
			buffer[copied] = character;
			buffer[copied + 1] = '\0';
			pipe_info->size = pipe_info->size - 1;
			copied++;
			pipe_info->read_end->current_pointer = get_next_address_circular(
					pipe_info->read_end->current_pointer);
		} else {
			if (is_write_end_dead(pipe_info)) {
				printf("write closed");
				return copied;
			}
			block_read_signal_write(pipe_info);
		}
	}
	pipe_info->write_end->ready = 1;
	return copied;
}

//// if write end is cloded and there is no characters in buffer no point of reading anymore
//int write_end_pipe_status(int fd, int status){
//	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
//	uint64_t i = 0;
//	int check = 0;
//	for(i = 0; i < MAX_FILES_OS; i++){
//		file_desc_t *current = global_fd_array[i].fd;
//		int count = global_fd_array[i].count;
//		if(current != NULL && ((current->flags &O_WRONLY) != 0) && count > 0){
//			if(((uint64_t)(current->current_pointer)& ~(0xfff)) == c_pointer){
//				current->ready = 1;
//				check = 1;
//				break;
//			}
//		}
//	}
//	if(check == 0){
//		return -1;
//	}
//	read_blocked_t *new_node = kmalloc(sizeof(read_blocked_t));
//	new_node->pid = currenttask->pid;
//	new_node->read_fd = fd;//local
//	new_node->write_fd = i;//global
//	new_node->read_write = 2; // read end is going to be blocked
//	new_node->next = NULL;
//	add_read_blocked_list(new_node);
//	fake_preempt(2);
//	__asm__ __volatile__("cli");
//	return 1;
//
//}
void read_thread_process(int fd, char *buffer, uint64_t size){
	file_desc_t *current_fd = currenttask->filearray[fd];
	int copied = 0;
	while(check_busy(current_fd));
	__asm__ __volatile__("cli");
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
	while((check_ready_read(current_fd) == 0)){
		fake_preempt(1);
	}
	__asm__ __volatile__("cli");
	if((int)(current_fd->size) != -999){
		copied = read_stdin(fd, size, buffer);
	}
	else{
		copied = handle_pipe_read(copied, size, current_fd, buffer);
	}

	int ppid = currenttask->ppid;
	task_struct_t *current_waiting = waitingtask;
	do {
		if (current_waiting != NULL) {
			if (current_waiting->pid == ppid) {
				regs_syscall_t *regs =
						(regs_syscall_t *) (current_waiting->state.kernel_rsp);
				regs->rax = copied;
				current_waiting->p_state = STATE_READY;
				break;
			}
		}
		if (current_waiting != NULL)
			current_waiting = current_waiting->next;
	}while(current_waiting!=waitingtask);
	if(current_fd == stdin_fd)
		clear_keyboard_busy();
	if(current_fd->posix_header == NULL)
		current_fd->ready = 0;
	current_fd->busy = 0;
	quit_kernel_thread();
}

//int read_end_pipe_status(int fd, int status){
//	uint64_t c_pointer = (uint64_t)(currenttask->filearray[fd]->current_pointer) & ~(0xfff);
//	uint64_t i = 0;
//	int check = 0;
//	for(i = 0; i < MAX_FILES_OS; i++){
//		file_desc_t *current = global_fd_array[i].fd;
//		int count = global_fd_array[i].count;
//		if(current != NULL && ((current->flags &~(2)) != 0) && count>0){ //~(2) puts checks no other flag permissions are on other than RD or RDWR
//			if(((uint64_t)(current->current_pointer)& ~(0xfff)) == c_pointer){
//				current->ready = 1;
//				check = 1;
//				break;
//			}
//		}
//	}
//	if(check == 0){
//		return -1;
//	}
//	read_blocked_t *new_node = kmalloc(sizeof(read_blocked_t));
//	new_node->pid = currenttask->pid;
//	new_node->read_fd = i;//global
//	new_node->write_fd = fd;//local
//	new_node->read_write = 1; // 1 is write end is blocked
//	new_node->next = NULL;
//	add_read_blocked_list(new_node);
//	fake_preempt(2);
//	__asm__ __volatile__("cli");
//	return 1;
//}

void write_thread_process(int fd, void *buffer, uint64_t size) {
	file_desc_t *current_fd = currenttask->filearray[fd];
	int printed = 0;
	while(check_busy(current_fd));
	__asm__ __volatile__("cli");
	reserve_fd(current_fd);

	if(currenttask->filearray[fd] == stdout_fd){
		printed = write_stdout(size, buffer);
	}
	else{
		printed = handle_pipe_write(size, current_fd, buffer);
	}

	int ppid = currenttask->ppid;
	task_struct_t *current_waiting = waitingtask;
	do{
		if(current_waiting!= NULL){
			if(current_waiting->pid == ppid){
				regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
				regs->rax = printed;
				current_waiting->p_state = STATE_READY;
				break;
			}
		}
		if(current_waiting != NULL)
			current_waiting = current_waiting->next;
	}while(current_waiting!=waitingtask);
	current_fd->busy = 0;
	quit_kernel_thread();
}
/////////////////////////////////////
int check_ready_read(file_desc_t *current_fd){
	return current_fd->ready;
}
int read_stdin(int fd, uint64_t size, char* buffer) {
	char* current_pointer = currenttask->filearray[fd]->current_pointer;
	int copied = 0;
	char* end_file = current_pointer + currenttask->filearray[fd]->size;
	char* buff = (char*) buffer;
	while (copied < size && current_pointer < end_file
			&& *current_pointer != '\0') {
		buff[copied] = *current_pointer;
		current_pointer++;
		(currenttask->filearray[fd]->current_pointer)++;
		copied++;
	}
	buff[copied] = '\0';
	return copied;
}
void block_read_signal_write(pipe_struct_t* pipe_info){
	pipe_info->read_end->ready = 0;
	currenttask->p_state = STATE_WAITING;
	pipe_info->write_end->ready = 1;
	read_blocked_t* new_node = kmalloc(sizeof(read_blocked_t));
	new_node->pid = currenttask->pid;
	new_node->pipe_info = pipe_info;
	new_node->read_write = 2;
	new_node->next = NULL;
	add_read_blocked_list(new_node);
	fake_preempt(2);//todo:fake preempt enable flag on stack
	__asm__ __volatile__("cli");
}
/////////////////////////////////////////
//clears keyoard buffer when a new process begins writing
void clear_keyboard_busy() {
	char *current = (char *) (input_buffer);
	for (uint64_t i = 0; i < 0x1000; i++) {
		*current = 0;
		current++;
	}
	currenttask->filearray[0]->current_pointer = (char *) global_keyboard;

}

void find_child(volatile task_struct_t *temp) {

	int ppid = temp->pid;
	int wait_for = temp->waiting_for;
	if (wait_for == -1) {
		task_struct_t *temp2 = waitingtask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->p_state != STATE_TERMINATED&& temp2->ppid == ppid && temp2->is_background != 1){
				return;
			}
			temp2 = temp2->next;
		} while (temp2 != waitingtask);
		temp2 = currenttask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->p_state != STATE_TERMINATED && temp2->ppid == ppid && temp2->is_background != 1){
				return;
			}
			temp2 = temp2->next;
		} while (temp2 != currenttask);
	} else {
		task_struct_t *temp2 = waitingtask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->p_state != STATE_TERMINATED && temp2->pid == wait_for){
				return;
			}
			temp2 = temp2->next;
		} while (temp2 != waitingtask);
		temp2 = currenttask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->p_state != STATE_TERMINATED&& temp2->pid == wait_for){
				return;
			}
			temp2 = temp2->next;
		} while (temp2 != currenttask);
	}
//	printf("daemon activated");
	temp->waiting_for = 999;
	temp->p_state = STATE_READY;
	printf("%d is back", temp->pid);
}

void check_user_process_waitpid_daemon_body() {
	volatile task_struct_t* temp = waitingtask;
	do {
		if (temp != NULL && temp->p_state == STATE_WAITING
				&& temp->is_kernel_process == 0 && temp->waiting_for != 999) {
			//				printf("found : %d", temp->pid);
			find_child(temp);
		}
		if (temp != NULL)
			temp = temp->next;
	} while (temp != waitingtask);
}

void check_user_process_waitpid_daemon() {

	while (1) {
		__asm__ __volatile__("cli");
		check_user_process_waitpid_daemon_body();
		fake_preempt(1);
	}
}
task_struct_t *check_unblock_waitq(int pid) {
	task_struct_t *temp = waitingtask;
	do {
		if (temp != NULL && temp->pid == pid) {
			return temp;
		}
		if (temp != NULL)
			temp = temp->next;
	} while (temp != waitingtask);
	return NULL;
}
int find_fd_from_struct(file_desc_t *filedesc, task_struct_t *task){
	for(uint64_t i = 0; i < MAX_FILES_SYSTEM; i++){
		if(task->filearray[i] == filedesc){
			return i;
		}
	}
	return -1;
}
void check_if_one_end_closed(volatile read_blocked_t *current){
	if(get_global_count_fd(current->pipe_info->write_end) <=1){
		current->pipe_info->read_end->ready = 1;
	}
	if(get_global_count_fd(current->pipe_info->read_end) <=1){
		current->pipe_info->write_end->ready = 1;
	}
}
void return_blocking_rw_to_runq_body() {

	volatile read_blocked_t* current = read_blocked_list;
	if (current != NULL) {
		do {

			task_struct_t* temp = NULL;
			if (current != NULL) {
				int pid = current->pid;
				temp = check_unblock_waitq(pid);
				check_if_one_end_closed(current);
			}
			if (temp != NULL && temp->p_state == STATE_WAITING) {
				if (current->read_write == 1) {
					int fd = find_fd_from_struct(current->pipe_info->write_end, temp);
					if (temp->filearray[fd]->ready == 1) {
						temp->p_state = STATE_READY;
						current = remove_read_blocked_list(current);
					}
				} else {
					int fd = find_fd_from_struct(current->pipe_info->read_end, temp);
					if (temp->filearray[fd]->ready == 1) {
						temp->p_state = STATE_READY;
						current = remove_read_blocked_list(current);
					}
				}
			}
			if (current != NULL)
				current = current->next;
		} while (current != NULL);
	}
}

void return_blocking_rw_to_runq() {
	while (1) {
		__asm__ __volatile__("cli");
		return_blocking_rw_to_runq_body();
		fake_preempt(1);
	}
}

void clean_up_process_body() {
	volatile task_struct_t* wait_task = waitingtask;
	if (wait_task != NULL) {
		volatile task_struct_t* current = waitingtask;
		volatile task_struct_t* prev = current;
		while (prev->next != current) {
			prev = prev->next;
		}
		do {
			if (current->p_state == STATE_TERMINATED) {
	//			volatile task_struct_t *to_be_removed = current;
				if (prev == current) {
					current = NULL;
					waitingtask = NULL;
				} else {
					prev->next = current->next;
					current = current->next;
				}
	//			cleanup_process(to_be_removed);
				//add function to clean up the to_be_removed task_struct
			} else {
				prev = current;
				current = current->next;
			}
		} while (current != NULL && current != waitingtask);
	}
}

void clean_up_processes() {
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
	while (1) {
		__asm__ __volatile__("cli");
		clean_up_process_body();
		fake_preempt(1);
	}
}

