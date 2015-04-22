#include <stdio.h>
#include<stdlib.h>
int arr[10];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	printf("temp!");
	int status;
	printf("waiting");
	waitpid(-1,&status, 0);
	printf("wait done");
//	while(1);
	return 0;
}
