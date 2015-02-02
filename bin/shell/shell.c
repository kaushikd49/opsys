#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define NULL ((void *)0)
#define ARG_LIMIT 1000

//Support changing current directory ( cd ) -
//Execute binaries interactively
//Execute scripts
//Execute pipelines of binaries ( /bin/ls | /bin/grep test )
//Set and use PATH and PS1 variables

int num_delims(char *str, char delim) {
	int i = 0, delimCount = 0;
	while (str[i] != '\0') {
		if (str[i] == delim)
			delimCount++;
		i++;
	}
	return delimCount;
}

char **tokenize(char *input, char delim) {
	int numDelims = num_delims(input, delim);
	char **tokens = (char**) malloc((numDelims + 2) * sizeof(char*));
	int i = 0, j = 0;
	while (input[i] != '\0') {
		if (i == 0 && input[i] != delim && input[i] != '\0') {
			tokens[j++] = input;
		}
		if (input[i] == delim && input[i + 1] != '\0') {
			tokens[j++] = input + (i + 1); // Found the start location of next token
			input[i] = '\0'; 	// Terminate previous token with '\0'
		}
		i++;
	}
	tokens[j] = NULL; 	//sentinel
	return tokens;
}

void take_action(char ** tokens) {
	int i = 0;
	printf("received tokens\n");
	while (tokens[i] != NULL) {
		printf("%s\n", tokens[i]);
		i++;
	}
}

int main(int argc, char* argv[], char* envp[]) {

	char input[ARG_LIMIT];
	while (1) {
		scanf(" %1000[^\n]", input);
//		printf("input is %s ", input);
		if (strcmp(input, "exit") == 0) {
			break;
		}
		char ** tokens = tokenize(input, ' ');
		take_action(tokens);
		free(tokens);
	}

	return 0;
}
