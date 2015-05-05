#include <stdio.h>
#include <stdlib.h>
int main(int argc, char* argv[], char* envp[]) {
	char *buffer = malloc(100);
	buffer = getcwd(buffer, 100);
	if(buffer != NULL){
		printf("\npwd: %s\n", buffer);
	}
	else{
		printf("\nerror with pwd\n");
	}
	return 0;
}
