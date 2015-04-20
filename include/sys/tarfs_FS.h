#ifndef __TARFS_FS_H
#define __TARFS_FS_H
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
enum {
	O_RDONLY = 0,
	O_WRONLY = 1,
	O_RDWR = 2,
	O_CREAT = 0x40,
	O_EXCL = 0x80,
	O_DIRECTORY = 0x10000,

};
uint64_t *find_file_tarfs(char *);
uint64_t open_tarfs(char *file_name, int flags);
uint64_t read_tarfs(int fd, void *buffer, uint64_t size);
void dents_tarfs(int fd, struct dirent *dirent_array, uint64_t size);
void init_tarfs();

#endif
