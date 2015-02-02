#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "shell_functions.h"
#define ARG_LIMIT 1000


//Support changing current directory ( cd ) -
//Execute binaries interactively
//Execute scripts
//Execute pipelines of binaries ( /bin/ls | /bin/grep test )
//Set and use PATH and PS1 variables

// Returns num of delims just based on their presence.
// Not escaping aware, which is fine for now as this method
// is used only to find the size of char ** tokens[] and the
// value returned will always be >= num of tokens actually
// generated.

int main(int argc, char* argv[], char* envp[]) {

	char input[ARG_LIMIT];
	while (1) {
		printf("shellzhere>> ");
		scanf(" %1000[^\n]", input);
//		printf("input is %s ", input);
		if (strcmp(input, "exit") == 0) {
			break;
		}
		char ** tokens = advance_tokenize(input, ' ', '"');
		take_action(tokens);
		free(tokens);
	}

	return 0;
}
