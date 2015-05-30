#include <stdio.h>
#include<stdlib.h>
int arr[100];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	char pathname[20] ="test/test.txt";
	int fd = open(pathname, O_RDONLY);
//	char *str = malloc(10000);
//	for(int i = 0; i < 10000; i++){
//		str[i] = '\0';
//	}
//	read(fd,str, 499);
//	close(fd);
//	write(1, str, 500);
	char *buffer = malloc(0x100);

	int ret = lseek(fd, 5, SEEK_SET);
	if(ret > 0){
		read(fd, buffer, 10);
		printf("1: %s\n", buffer);
	}
	ret = lseek(fd, -5, SEEK_CUR);
	if(ret > 0){
			read(fd, buffer, 15);
			printf("1: %s\n", buffer);
	}
	ret = lseek(fd, 20, SEEK_END);
	if(ret > 0){
		read(fd, buffer, 15);
		printf("1: %s\n", buffer);
	}
	return 0;
}
