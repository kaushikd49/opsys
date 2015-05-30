#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
	printf("\n");
	for (int i = 1; i < argc; i++) {
		printf("%s", argv[i]);
	}
	printf("\n");
}
