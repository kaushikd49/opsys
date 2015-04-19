#ifndef __TARFS_FS_H
#define __TARFS_FS_H
uint64_t *find_file_tarfs(char *);
uint64_t open_tarfs(char *file_name, int flags);
uint64_t read_tarfs(int fd, void *buffer, uint64_t size);
void init_tarfs();
enum {
	O_RDONLY = 0,
	O_WRONLY = 1,
	O_RDWR = 2,
	O_CREAT = 0x40,
	O_EXCL = 0x80,
	O_DIRECTORY = 0x10000,

};
#endif
