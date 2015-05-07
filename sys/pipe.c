//struct read_blocked{
//	int pid;
//	pipe_struct_t *pipe_info;
//	int read_write; // which end of the pipe is blocked
//	struct read_blocked *next;
//};
//typedef struct read_blocked read_blocked_t;
//read_blocked_t *read_blocked_list = NULL;
//pipe_struct_t *find_pipe_from_write(file_desc_t *fd_write){
//	pipe_struct_t *current = pipe_list;
//	if(current == NULL){
//		return NULL;
//	}
//	while(current!=NULL){
//		if(current->write_end == fd_write){
//			return current;
//		}
//		current = current->next;
//	}
//	return NULL;
//}
//
//pipe_struct_t *find_pipe_from_read(file_desc_t *fd_read){
//	pipe_struct_t *current = pipe_list;
//	if(current == NULL){
//		return NULL;
//	}
//	while(current!=NULL){
//		if(current->read_end == fd_read){
//			return current;
//		}
//		current = current->next;
//	}
//	return NULL;
//}
//
//int check_busy(file_desc_t *current_fd){
//	int ans = (current_fd->busy) & (current_fd->current_process != currenttask->pid);
//	return current_fd->busy;
//}
//
//void reserve_fd(file_desc_t* current_fd) {
//	current_fd->busy = 1;
//	current_fd->current_process = currenttask->pid;
//}
//
//int write_stdout(uint64_t size, void* buffer) {
//	int printed = 0;
//	char* print_buffer = (char*) buffer;
//	while (printed < size && print_buffer[printed] != '\0') {
//		char character = print_buffer[printed];
//		write_char_to_vid_mem(character, 0);
//		printed++;
//	}
//	write_char_to_vid_mem('\0', 0);
//	return printed;
//}
//
//int get_pipe_size(pipe_struct_t* pipe_info) {
//	return pipe_info->size;
//}
//void *get_next_address_circular(void *addr){
//	uint64_t addr_uint = (uint64_t)addr;
//	uint64_t addr_mod = addr_uint | 0xfff;
//	if(addr_uint == addr_mod){
//		addr_uint = addr_uint &~(0xfff);
//	}
//	else{
//		addr_uint++;
//	}
//	return (void *)addr_uint;
//}
//void add_read_blocked_list(read_blocked_t *node){
//	if(read_blocked_list == NULL){
//		read_blocked_list = node;
//		return;
//	}
//	node->next = read_blocked_list;
//	read_blocked_list = node;
//}
//
//void block_write_signal_read(pipe_struct_t* pipe_info) {
//	pipe_info->write_end->ready = 0;
//	currenttask->p_state = STATE_WAITING;
//	pipe_info->read_end->ready = 1;
//	read_blocked_t* new_node = kmalloc(sizeof(read_blocked_t));
//	new_node->pid = currenttask->pid;
//	new_node->pipe_info = pipe_info;
//	new_node->read_write = 1;
//	new_node->next = NULL;
//	add_read_blocked_list(new_node);
//	fake_preempt(2);
//	asm("cli");
//}
//
//void is_read_end_dead(pipe_struct_t* pipe_info) {
//	int count = 0;
//	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
//		file_desc_t* current = global_fd_array[i].fd;
//		int count = global_fd_array[i].count;
//		if (current == pipe_info->read_end && count == 1) {
//			count = 1;
//		}
//	}
//	return count;
//}
//
//void is_write_end_dead(pipe_struct_t* pipe_info) {
//	int count = 0;
//	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
//		file_desc_t* current = global_fd_array[i].fd;
//		int count = global_fd_array[i].count;
//		if (current == pipe_info->write_end && count == 1) {
//			count = 1;
//		}
//	}
//	return count;
//}
//
//int handle_pipe_write(uint64_t size, file_desc_t* current_fd, void* buffer) {
//	pipe_struct_t* pipe_info = find_pipe_from_write(current_fd);
//	char* copy_buf = (char*) buffer;
//	while (printed < size && copy_buf[printed] != '\0') {
//		int size = get_pipe_size(pipe_info);
//		if (size < 0x1000) {
//			char character = copy_buf[printed];
//			char* copy_to = (char*) (pipe_info->write_end->current_pointer);
//			*copy_to = character;
//			pipe_info->size = pipe_info->size + 1;
//			pipe_info->write_end->current_pointer = get_next_address_circular(
//					pipe_info->write_end->current_pointer);
//		} else {
//			block_write_signal_read(pipe_info);
//		}
//		int read_status = is_read_end_dead(pipe_info);
//		if(read_status == 1){
//			break;
//		}
//	}
//	return printed;
//}
//
//void write_thread_process(int fd, void *buffer, uint64_t size){
//	file_desc_t *current_fd = currenttask->filearray[fd];
//	int printed;
//	while(check_busy(current_fd));
//	__asm__ __volatile__("cli");
//	reserve_fd(current_fd);
//
//	if(currenttask->filearray[fd] == stdout_fd){
//		printed = write_stdout(size, buffer);
//	}
//	else{
//		printed = handle_pipe_write(size, current_fd, buffer);
//	}
//	pipe_info->read_end->ready = 1;
//	int ppid = currenttask->ppid;
//	task_struct_t *current_waiting = waitingtask;
//	do{
//		if(current_waiting!= NULL){
//			if(current_waiting->pid == ppid){
//				regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
//				regs->rax = printed;
//				current_waiting->p_state = STATE_READY;
//				break;
//			}
//		}
//		if(current_waiting != NULL)
//			current_waiting = current_waiting->next;
//	}while(current_waiting!=waitingtask);
//	current_fd->busy = 0;
//	quit_kernel_thread();
//}
//
//int check_ready_read(file_desc_t *current_fd){
//	return current_fd->ready;
//}
//
//int read_stdin(int fd, uint64_t size, char* buffer) {
//	char* current_pointer = currenttask->filearray[fd]->current_pointer;
//	int copied = 0;
//	char* end_file = current_pointer + currenttask->filearray[fd]->size;
//	char* buff = (char*) buffer;
//	while (copied < size && current_pointer < end_file
//			&& *current_pointer != '\0') {
//		buff[copied] = *current_pointer;
//		current_pointer++;
//		(currenttask->filearray[fd]->current_pointer)++;
//		copied++;
//	}
//	buff[copied] = '\0';
//	return copied;
//}
//
//void block_read_signal_write(pipe_struct_t* pipe_info){
//	pipe_info->read_end->ready = 0;
//	currenttask->p_state = STATE_WAITING;
//	pipe_info->write_end->ready = 1;
//	read_blocked_t* new_node = kmalloc(sizeof(read_blocked_t));
//	new_node->pid = currenttask->pid;
//	new_node->pipe_info = pipe_info;
//	new_node->read_write = 2;
//	new_node->next = NULL;
//	add_read_blocked_list(new_node);
//	fake_preempt(2);todo:fake preempt enable flag on stack
//	asm("cli");
//}
//void read_thread_process(int fd, char *buffer, uint64_t size){
//	file_desc_t *current_fd = currenttask->filearray[fd];
//	int copied = 0;
//	while(check_busy(current_fd));
//	__asm__ __volatile__("cli");
//	current_fd->busy = 1;
//	current_fd->current_process = currentask->pid;
//	while(check_ready_read(current_fd) == 0){
//		fake_preempt(1);
//	}
//	__asm__ __volatile__("cli");
//	if((int)(current_fd->size) != -999){
//		copied = read_stdin(fd, size, buffer);
//	}
//	else{
//		pipe_struct_t* pipe_info = find_pipe_from_read(current_fd);
//		while(copied < size-1){
//			int size = get_pipe_size(pipe_info);
//			if(size > 0){
//				char character = *(pipe_info->read_end->current_pointer);
//				buffer[copied] = character;
//				buffer[copied+1] = '\0';
//				pipe_info->size = pipe_info->size - 1;
//				pipe_info->read_end->current_pointer = get_next_address_circular(
//									pipe_info->read_end->current_pointer);
//			}
//			else{
//				if(is_write_end_dead(pipe_info)){
//					break;
//				}
//				block_read_signal_write(pipe_info);
//			}
//		}
//	}
//	pipe_info.write_end.ready = 1;
//	int ppid = currenttask->ppid;
//	task_struct_t *current_waiting = waitingtask;
//	do{
//		if(current_waiting!=NULL){
//			if(current_waiting->pid == ppid){
//				regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
//				regs->rax = copied;
//				current_waiting->p_state = STATE_READY;
//				break;
//			}
//		}
//		if(current_waiting!=NULL)
//			current_waiting = current_waiting->next;
//	}while(current_waiting!=waitingtask);
//
//	current_fd->busy = 0;
//	quit_kernel_thread();
//}
//
