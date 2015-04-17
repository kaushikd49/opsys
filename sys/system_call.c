#include<sys/system_calls.h>
#include<sys/sbunix.h>
#include<sys/system_calls.h>
#include<sys/fork.h>

int write_system_call(int fd, const void *buff, size_t count) {
	if (fd != 1) { //EACCES error
		return -13;
	}
//	if(count>PAGE_SIZE){//cannot write more than a page at a time
//		return -27;
//	}
	char *print_buffer = (char *) buff;
	int printed = 0;
	while (printed < count && print_buffer[printed] != '\0') {
		char character = print_buffer[printed];
		write_char_to_vid_mem(character, 0);
		printed++;
	}
	return printed;
}

int fork_sys_call() {
	return do_fork();
}
