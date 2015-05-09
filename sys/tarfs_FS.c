#include<sys/defs.h>
#include<sys/sbunix.h>
#include <sys/kmalloc.h>
#include<sys/kmalloc.h>
#include<sys/tarfs.h>
#include<sys/tarfs_FS.h>
#include<sys/process.h>
#include<sys/scheduling.h>
#include<errno.h>
#include<sys/freelist.h>
#include<sys/paging.h>
#include<sys/system_calls.h>
#include<sys/isr_stuff.h>
pipe_struct_t *pipe_list = NULL;
uint64_t *find_file_tarfs(char *file_name){
	struct posix_header_ustar *current =
			(struct posix_header_ustar *) &_binary_tarfs_start;
	int i =0;
	while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
		if(strcmp(file_name, current->name) == 0){
			//printf("%s %s: found file", current->name, current->magic);
			return (uint64_t *)current;
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
	return NULL;
}

int get_free_fd(){
	for(uint64_t i = 0;i<MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] == NULL){
			currenttask->filearray[i] = (file_desc_t *)kmalloc(sizeof(file_desc_t));
			int ret = add_to_global_fd(currenttask->filearray[i]);
			if(ret == 0){
				return -1;
			}
			currenttask->filearray[i]->used = 1;
			currenttask->filearray[i]->posix_header = NULL;
			currenttask->filearray[i]->current_pointer = NULL;
			currenttask->filearray[i]->flags = -1;
			currenttask->filearray[i]->size = -1;
			currenttask->filearray[i]->busy = -1;
					currenttask->filearray[i]->current_process = -1;
			currenttask->filearray[i]->ready = -1;
			return i;
		}
	}
	return -1;
}

void return_fd(int fd){
	if(currenttask->filearray[fd] !=NULL){
		decrement_global_count_fd(currenttask->filearray[fd]);
		currenttask->filearray[fd] = NULL;
	}
}

uint64_t open_tarfs(char *file_name, int flags){
	if(!(flags == O_RDONLY || flags == (O_RDONLY|O_DIRECTORY) || flags == O_WRONLY || flags == O_RDWR)){
		return -EINVAL;
	}
	int fd = -1;
	if(strcmp(file_name, "") == 0){
//		printf("hereee");
		if((flags &O_DIRECTORY) == 0)
			return -ENOTDIR;
		else{
			fd = get_free_fd();
			currenttask->filearray[fd]->current_pointer = NULL;
			currenttask->filearray[fd]->posix_header = NULL;
			currenttask->filearray[fd]->flags = flags;
			currenttask->filearray[fd]->busy = 0;
			currenttask->filearray[fd]->current_process = -1;
			currenttask->filearray[fd]->ready = 1;
		}

	}
	else{

		uint64_t *tarfs_header = find_file_tarfs(file_name);

		if(tarfs_header == NULL){
			char temp_str[100];
			for(uint64_t i = 0; i < 100; i++){
				temp_str[i] = '\0';
			}
			strcpy(temp_str, currenttask->pwd);
			int len = strlen(temp_str);
			strcpy(temp_str+len, file_name);
			tarfs_header = find_file_tarfs(temp_str);
			if(tarfs_header == NULL)
				return -1;
		}

		fd = get_free_fd();
		if(fd == -1){
			return -EMFILE;
		}
//		struct posix_header_ustar *header = (struct posix_header_ustar *)tarfs_header;
//		char five = '5';
//		if((((flags)&(O_DIRECTORY)) != 0) && *(header->typeflag) != five){
//			return -ENOTDIR;
//		}
		currenttask->filearray[fd]->current_pointer = (char *)((uint64_t)tarfs_header + (uint64_t)(sizeof(struct posix_header_ustar)));
		struct posix_header_ustar *temp = (struct posix_header_ustar *)tarfs_header;
		currenttask->filearray[fd]->posix_header = temp;
		currenttask->filearray[fd]->size = convert_ocatalstr_todecimal(temp->size);
		currenttask->filearray[fd]->flags = flags;
		currenttask->filearray[fd]->busy = 0;
		currenttask->filearray[fd]->current_process = -1;
		currenttask->filearray[fd]->ready = 1;

	}
	return fd;
}

//uint64_t read_tarfs(int fd, void *buffer, uint64_t size){
//	char *current_pointer = currenttask->filearray[fd]->current_pointer;
//	char *end_file = current_pointer + currenttask->filearray[fd]->size;
//	int copied = 0;
//	char *buff = (char *)buffer;
//	while(copied < size && current_pointer < end_file ){
//		buff[copied] = *current_pointer;
//		current_pointer++;
//		currenttask->filearray[fd]->current_pointer++;
//		copied++;
//	}
//	return copied;
//}

uint64_t read_tarfs(int fd, void *buffer, uint64_t size, uint64_t stack_top){
	//change the process state to waiting
	if(currenttask->filearray[fd] ==NULL){
		regs_syscall_t *regs = (regs_syscall_t *)(stack_top);
		regs->rax = -EBADF;
		return stack_top;
	}
	else if((currenttask->filearray[fd]->flags &O_DIRECTORY) != 0 ){
		regs_syscall_t *regs = (regs_syscall_t *)(stack_top);
		regs->rax = -EISDIR;
		return stack_top;
	}
	else if((currenttask->filearray[fd]->flags != O_RDONLY) && (currenttask->filearray[fd]->flags != O_RDWR)){
		regs_syscall_t *regs = (regs_syscall_t *)(stack_top);
		regs->rax = -EINVAL;
		return stack_top;
	}
	return temp_preempt_wait(fd, buffer, size, stack_top);
}
uint64_t write_syscall(int fd, void *buffer, uint64_t size, uint64_t stack_top){
	if(currenttask->filearray[fd] ==NULL){
		regs_syscall_t *regs = (regs_syscall_t *)(stack_top);
		regs->rax = -EBADF;
		return stack_top;
	}
	else if((currenttask->filearray[fd]->flags != O_WRONLY) && (currenttask->filearray[fd]->flags != O_RDWR)){
		regs_syscall_t *regs = (regs_syscall_t *)(stack_top);
		regs->rax = -EINVAL;
		return stack_top;
	}
	return temp_preempt_write(fd, buffer, size, stack_top);
}
int str_prefix(char *prefix, char *str){
	int current = 0;
	while(prefix[current] !='\0'&& str[current] !='\0' && prefix[current] == str[current]){
		current++;
	}
	if(prefix[current] == '\0')
		return 0;
	return -1;
}
int is_root_tarfs_folder(char *name){
	int i = 0;
	while(name[i] != '\0'){
		if(name[i] == '/'){
			if(name[i+1] == '\0' || name[i+1] == ' '){
//				printf("%s", name);
				return 1;
			}
			else{
				return 0;
			}
		}
		i++;
	}
	return 0;

}
void find_and_populate_dirent_array(struct dirent *dirent_array, uint64_t size, char *dir_name){
	if(dir_name == NULL){
		struct posix_header_ustar *current =
				(struct posix_header_ustar *) &_binary_tarfs_start;
		int i =0;
		int dirent_offset = 0;
		while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
			if(is_root_tarfs_folder(current->name) == 1){
				if(((uint64_t)dirent_array + (dirent_offset + 1)*(sizeof(struct dirent))) < ((uint64_t)dirent_array + size)){
					dirent_array[dirent_offset].d_ino = 99;
					strcpy(dirent_array[dirent_offset].d_name, current->name);
					dirent_array[dirent_offset].d_off = 99;
					//					if(dirent_offset > 0)
					dirent_array[dirent_offset].d_reclen = sizeof(struct dirent);
					//					dirent_array[dirent_offset].d_reclen = 0;
					dirent_offset++;
				}
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
		dirent_array[dirent_offset].d_reclen = 0;
	}
	else{
		struct posix_header_ustar *current =
				(struct posix_header_ustar *) &_binary_tarfs_start;
		int i =0;
		int dirent_offset = 0;
		while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
			if(str_prefix(dir_name, current->name) == 0){
				if(((uint64_t)dirent_array + (dirent_offset + 1)*(sizeof(struct dirent))) < ((uint64_t)dirent_array + size)){
					dirent_array[dirent_offset].d_ino = 99;
					strcpy(dirent_array[dirent_offset].d_name, current->name);
					dirent_array[dirent_offset].d_off = 99;
					//					if(dirent_offset > 0)
					dirent_array[dirent_offset].d_reclen = sizeof(struct dirent);
					//					dirent_array[dirent_offset].d_reclen = 0;
					dirent_offset++;
				}
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
		dirent_array[dirent_offset].d_reclen = 0;
	}
//	printf("dirent:%d", dirent_offset);
}
void dents_tarfs(int fd, struct dirent *dirent_array, uint64_t size){
	if((currenttask->filearray[fd]->flags & O_DIRECTORY) == 0){
		return;
	}
	struct posix_header_ustar *temp = currenttask->filearray[fd]->posix_header;
	if(temp == NULL){
		find_and_populate_dirent_array(dirent_array, size, NULL);
	}
	else{
		char *dir_name = temp->name;
		find_and_populate_dirent_array(dirent_array, size, dir_name);
	}
}
int close_tarfs(int fd){
	if(fd<0 || fd >= 50){
		return -1;
	}
	if(currenttask->filearray[fd] != NULL){
		decrement_global_count_fd(currenttask->filearray[fd]);
		currenttask->filearray[fd]= NULL;
		return 0;
	}
	return -1;
}

int dup_tarfs(int fd){
	for(uint64_t i = 0;i<MAX_FILES_SYSTEM; i++){
		if(currenttask->filearray[i] == NULL){

			currenttask->filearray[i] = currenttask->filearray[fd];
			increment_global_count_fd(currenttask->filearray[i]);
			return i;
			break;
		}
	}
	return -1;
}

int dup2_tarfs(int fd_old, int fd_new){
	if(currenttask->filearray[fd_old]!=NULL && currenttask->filearray[fd_new]!=NULL){
		decrement_global_count_fd(currenttask->filearray[fd_new]);
		currenttask->filearray[fd_new] = currenttask->filearray[fd_old];
		increment_global_count_fd(currenttask->filearray[fd_new]);
		return fd_new;
	}
	return -1;
}

void add_to_pipe_list(pipe_struct_t *node){
	if(pipe_list == NULL){
		pipe_list = node;
		return;
	}
	node->next = pipe_list;
	pipe_list = node;

}

int pipe_tarfs(int pipe[2]){
	int pipe_read = get_free_fd();
	int pipe_write = get_free_fd();
	if( pipe_read == -1 || pipe_write == -1){
		if(pipe_read != -1){
			close_tarfs(pipe_read);
		}
		if(pipe_write != -1){
			close_tarfs(pipe_write);
		}
		return -EMFILE;
	}
	pipe[0] = pipe_read;
	pipe[1] = pipe_write;
	void *free_frame = (void *) get_free_frames(0);
	void *virtual_addr = (void *) get_virtual_location(0);
	setup_kernel_page_tables((uint64_t) virtual_addr,
					(uint64_t) free_frame);
	void *pipe_buffer = virtual_addr;
	currenttask->filearray[pipe_write]->current_pointer = (char *)pipe_buffer;
	currenttask->filearray[pipe_write]->flags = O_WRONLY;
	currenttask->filearray[pipe_write]->size = -999;//differentiates pipe from a file buffer
	currenttask->filearray[pipe_write]->used = 1;
	currenttask->filearray[pipe_write]->busy = 0;
	currenttask->filearray[pipe_write]->current_process = currenttask->pid;
	currenttask->filearray[pipe_write]->ready = 1;
	currenttask->filearray[pipe_read]->current_pointer = (char *)pipe_buffer;
	currenttask->filearray[pipe_read]->flags = O_RDONLY;
	currenttask->filearray[pipe_read]->size = -999;
	currenttask->filearray[pipe_read]->used = 1;
	currenttask->filearray[pipe_read]->busy = 0;
	currenttask->filearray[pipe_read]->current_process = currenttask->pid;
	currenttask->filearray[pipe_read]->ready = 0;
	struct pipe_struct *new_node = kmalloc(sizeof(struct pipe_struct));
	new_node->read_end = currenttask->filearray[pipe_read];
	new_node->write_end = currenttask->filearray[pipe_write];
	new_node->size = 0;
	new_node->next = NULL;
	add_to_pipe_list(new_node);
//	increment_global_count_fd(new_node->read_end);
//	increment_global_count_fd(new_node->write_end);
	return 0;
}

