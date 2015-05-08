#include <stdio.h>
#include<stdlib.h>

int arr[100];
int b = 10;
int main(int argc, char* argv[], char* envp[]) {
//	char * ptr = (char *) malloc(100);
	printf("should fault now \n");
//	*(ptr + 0x2000) = 'a';
	*((char *) (0xffffffffb0000000)) = 'a';

	return 0;
}
