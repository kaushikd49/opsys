#include<sys/defs.h>
#include<sys/sbunix.h>
#include <sys/kmalloc.h>
#include<sys/kmalloc.h>
#include<sys/tarfs.h>
#include<sys/tarfs_FS.h>
#include<sys/process.h>
#define MAX_FILES_SYSTEM 50
#define NAME_MAX 255
#define MAX_DIR 200
typedef struct{
	int fd;
	struct posix_header_ustar *posix_header;
	char *current_pointer;
	uint64_t flags;
	uint64_t size;
	int used;
}file_desc_t;
struct dirent {
	long d_ino;
	uint64_t d_off;
	unsigned short d_reclen;
	char d_name[NAME_MAX + 1];
};
file_desc_t *fd_list = NULL;
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


void init_tarfs(){
	fd_list = (file_desc_t *)kmalloc(MAX_FILES_SYSTEM*sizeof(file_desc_t));
	for(uint64_t i = 0; i < MAX_FILES_SYSTEM; i++){
		fd_list[i].fd =i;
		fd_list[i].used = 0;
		fd_list[i].current_pointer = NULL;
	}
	fd_list[0].used = 1;
	fd_list[1].used = 1;
	fd_list[2].used = 1;
}

int get_free_fd(){
	for(uint64_t i = 0;i<MAX_FILES_SYSTEM; i++){
		if(fd_list[i].used == 0){
			fd_list[i].used = 1;
			return fd_list[i].fd;
		}
	}
	return -1;
}

void return_fd(int fd){
	if(fd == 0 || fd == 1 || fd == 2){
		return;
	}
	fd_list[fd].used = 0;
}

uint64_t open_tarfs(char *file_name, int flags){
	if(!(flags == O_RDONLY || flags == (O_RDONLY|O_DIRECTORY))){
		return -1;
	}
	int fd = get_free_fd();
	uint64_t *tarfs_header = find_file_tarfs(file_name);
	if(tarfs_header == NULL){
		return -1;
	}
	fd_list[fd].current_pointer = (char *)((uint64_t)tarfs_header + (uint64_t)(sizeof(struct posix_header_ustar)));
	struct posix_header_ustar *temp = (struct posix_header_ustar *)tarfs_header;
	fd_list[fd].posix_header = temp;
	fd_list[fd].size = convert_ocatalstr_todecimal(temp->size);
	fd_list[fd].flags = flags;
	printf("\nopening file of size %d", fd_list[fd].size);
	return fd;
}

uint64_t read_tarfs(int fd, void *buffer, uint64_t size){
	char *current_pointer = fd_list[fd].current_pointer;
	char *end_file = current_pointer + fd_list[fd].size;
	int copied = 0;
	char *buff = (char *)buffer;
	while(copied < size && current_pointer < end_file ){
		buff[copied] = *current_pointer;
		current_pointer++;
		fd_list[fd].current_pointer++;
		copied++;
	}
	return copied;
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
					if(dirent_offset > 0)
						dirent_array[dirent_offset-1].d_reclen = sizeof(struct dirent);
					dirent_array[dirent_offset].d_reclen = 0;
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
}
void dents_tarfs(int fd, struct dirent *dirent_array, uint64_t size){
	if((fd_list[fd].flags & O_DIRECTORY) == 0){
		return;
	}
	struct posix_header_ustar *temp = fd_list[fd].posix_header;
	char *dir_name = temp->name;
	find_and_populate_dirent_array(dirent_array, size, dir_name);
}
void close_tarfs(int fd){
	fd_list[fd].used  = 0;
	fd_list[fd].current_pointer = NULL;
}

