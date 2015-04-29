#include <stdio.h>
#include<stdlib.h>
int arr[10];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	printf("temp!");
//	int status;
	printf("waiting");
//	waitpid(-1,&status, 0);

//	sleep(7);
//	int fd = dup(1);
//	char *buff = "dup test";
//	printf("fd: %d\n", fd);
//	int count = write(fd, buff, 11);
//	printf("wait done: %d", count);
//	fd = dup2(0, fd);
//	char buffer[10];
//	read(fd, buffer, 10);
//	printf("%s", buffer);
//	while(1);
	execve("bin/hello",argv, envp);
	printf("ans: %d %s %s", argc, argv[0], envp[0]);
	return 0;
}
