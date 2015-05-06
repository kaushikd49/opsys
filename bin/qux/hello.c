#include <stdio.h>
#include<stdlib.h>

int arr[100];
int b =10;
int main(int argc, char* argv[], char* envp[]) {
	printf("Hello World!\n");
	char *str = malloc(100);
	printf("Should seg fault now \n");
	printf("OO \n", str[300000]);
//	while(1);
	return 0;
}
