#include <stdio.h>
#include<stdlib.h>
#include<errno.h>
int arr[100];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	uint64_t old_brk = get_brk(0);
	char *str = malloc(100000000);
	if(str == NULL){
		printf("cannot malloc so much hitting stack");

	}
	str[5000] = 'c';
	str[5001] = 'a';
	str[5002] = 't';
	str[5003] = '\0';
	int x = getpid();
	int y = getppid();
	uint64_t new_brk = get_brk(0);
	sleep(10);
	int fd = open("test/test.txt", O_WRONLY);
	if(fd == -1){
		//		int backupErrno = errno;
		printf("error: %d", errno);
	}
	int rd = read(fd, str, 100);
	if(rd == -1){
		printf("error: %d", errno);
	}
	printf("\n %x - %s %x - %x %x - %x \n",str, str+5000, x, y, old_brk, new_brk);
	return 0;
}
