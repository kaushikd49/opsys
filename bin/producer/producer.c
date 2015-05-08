#include<stdio.h>
#include<stdlib.h>
int main(){
	int fd = open("test/test.txt", O_RDONLY);
	if(fd <0){
		char str[20] = "error";
		write(2, str, 10);
	}
	char *buffer = malloc(0x1000);
	int count = read(fd, buffer, 0x1000);
	while(count> 0){
		write(1, buffer, 0x1000);
		if(count < 0x1000){
			break;
		}
		count = read(fd, buffer, 0x1000);
	}
	close(fd);
}
