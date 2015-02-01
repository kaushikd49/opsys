#include <stdio.h>
#define NULL ((void *)0)
int main(int argc, char* argv[], char* envp[]) {
	int i=0;
	printf("Hello World!\n");
	while(envp[i]!=NULL){
		printf("%d - %s\n",i,envp[i]);
		i++;
	}
	return 0;
}
