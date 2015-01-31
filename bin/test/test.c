#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[], char *envp[]){
	execve("ls", NULL, NULL);	
	printf("error");	
	return 0;

}
