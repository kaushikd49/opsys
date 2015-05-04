#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[], char *envp[]){
//	printf("inside wcc");
	char buffer[20];
//	scanf("%s", buffer);
	read(0, buffer, 1000);
	printf("back");
	char *start = buffer;
	int count = 0;
	while(*start !='\0' && count < 1000){
		count++;
		start++;
	}
	printf("\ncount s: %d\n",count);
}
