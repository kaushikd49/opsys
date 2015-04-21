#include<sys/defs.h>
#include<sys/sbunix.h>
#include <sys/kmalloc.h>
#include<sys/kmalloc.h>
#include<sys/tarfs.h>
#include<sys/tarfs_FS.h>
#include<sys/process.h>
#include<sys/scheduling.h>
uint64_t *find_file_tarfs(char *file_name){
	struct posix_header_ustar *current =
			(struct posix_header_ustar *) &_binary_tarfs_start;
	int i =0;
	while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
		if(strcmp(file_name, current->name) == 0){
			printf("%s %s: found file", current->name, current->magic);
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
	currenttask->filearray[fd] = NULL;
}

uint64_t open_tarfs(char *file_name, int flags){
	if(!(flags == O_RDONLY || flags == (O_RDONLY|O_DIRECTORY) || flags == O_WRONLY)){
		return -1;
	}
	int fd = get_free_fd();
	uint64_t *tarfs_header = find_file_tarfs(file_name);
	if(tarfs_header == NULL){
		return -1;
	}
	currenttask->filearray[fd]->current_pointer = (char *)((uint64_t)tarfs_header + (uint64_t)(sizeof(struct posix_header_ustar)));
	struct posix_header_ustar *temp = (struct posix_header_ustar *)tarfs_header;
	currenttask->filearray[fd]->posix_header = temp;
	currenttask->filearray[fd]->size = convert_ocatalstr_todecimal(temp->size);
	currenttask->filearray[fd]->flags = flags;
	currenttask->filearray[fd]->busy = 0;
	currenttask->filearray[fd]->current_process = -1;
	currenttask->filearray[fd]->ready = 1;
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
	return temp_preempt_wait(fd, buffer, size, stack_top);
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

void find_and_populate_dirent_array(struct dirent *dirent_array, uint64_t size, char *dir_name){
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
	printf("dirent:%d", dirent_offset);
}
void dents_tarfs(int fd, struct dirent *dirent_array, uint64_t size){
	if((currenttask->filearray[fd]->flags & O_DIRECTORY) == 0){
		return;
	}
	struct posix_header_ustar *temp = currenttask->filearray[fd]->posix_header;
	char *dir_name = temp->name;
	find_and_populate_dirent_array(dirent_array, size, dir_name);
}
void close_tarfs(int fd){
	currenttask->filearray[fd]= NULL;
}

