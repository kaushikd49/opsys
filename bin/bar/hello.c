#include <stdio.h>
#include<stdlib.h>
int arr[100];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	char pathname[20] ="test/test.txt";
	int fd = open(pathname, O_RDONLY);
	char *str = malloc(10000);
	for(int i = 0; i < 10000; i++){
		str[i] = '\0';
	}
	read(fd,str, 5000);
	close(fd);
	write(1, str, 5000);
	return 0;
}
