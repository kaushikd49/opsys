#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "shell_functions.h"
#define ARG_LIMIT 1000

void handleChildExec(char **tokens, char** envpp) {
	int status;
	pid_t child = fork();
	if (child >= 0) {
		if (child == 0) {
			char pathPrefix[250];
			char *current, *next;
			// this is what Input has to change.execve(newargs[0], newargs, NULL);
			execve(tokens[0], tokens, envpp);
			char* path = getEnv("PATH=", envpp);
			current = path;
			//printf("%s",path);
			while (1) {
				next = strchr(current, ':');
				if (next != NULL) {
					strcpybtwptrs(current, next, pathPrefix);
					strcat(pathPrefix, "/");
					strcat(pathPrefix, tokens[0]);
					execve(pathPrefix, tokens, envpp);
					current = next + 1;
				} else {
					printf("command not found\n");
					break;
				}
			}
		} else {
			waitpid(-1, &status, 0);
		}
	} else {
		printf("internal error unsuccessful fork");
		exit(0);
	}
}

void handleGetenv(char **tokens, char *envpp[]) {
	char *cmd = tokens[1];
	char* getenvOutput = getEnv(cmd, envpp);
	if (getenvOutput != NULL)
		printf("%s\n", getenvOutput + 1);
	else
		printf("\n");
}

char ** handleSetenv(char **tokens, char *envpp[]) {
	char *cmd = tokens[1], *envStr = tokens[2]; //todo validations
	int size = strlen(cmd) + strlen(envStr) + 2;
	char* newStr = (char*) malloc(size * sizeof(char));
	strcpy(newStr, cmd);
	strcat(newStr, "=");
	strcat(newStr, envStr);
	envpp = setEnv(newStr, envpp);
	return envpp;
}

char ** take_action(char** tokens, char *envpp[]) {
	print_tokens(tokens);
	char *cmd = tokens[0];
	if (strcmp("setenv", cmd) == 0) {
		envpp = handleSetenv(tokens, envpp);
	} else if (strcmp("getenv", cmd) == 0) {
		handleGetenv(tokens, envpp);
	} else if (strcmp("cd", cmd) == 0) {
		chdir(tokens[1]); //todo: return status
	} else {
		handleChildExec(tokens, envpp);
	}
	return envpp;
}

char** cmd_line_arg_case(char input[ARG_LIMIT], char* argv[], char* envpp[]) {
	int flag = 1;
	//todo: check for size greater than ARG_LIMIT
	int fileHandle = open(argv[1], O_RDONLY);
	do {
		flag = read_line(input, fileHandle);
		char** tokens = advance_tokenize(input, ' ', '"');
		if (tokens[0] == NULL || strncmp(tokens[0], "#", 1) == 0)
			//todo: check for comments in the middle
			continue;

		envpp = take_action(tokens, envpp);
	} while (flag == 1);
	close(fileHandle);
	return envpp;
}

char** interactive_case(char input[ARG_LIMIT], char* envpp[]) {
	char ps1[] = "PS1=prompt>>";
	envpp = setEnv(ps1, envpp);
	while (1) {
		printf("%s", getEnv("PS1=", envpp));
		scanf(" %1000[^\n]", input);
		if (strcmp(input, "exit") == 0) {
			break;
		}
		char** tokens = advance_tokenize(input, ' ', '"');
		envpp = take_action(tokens, envpp);
		free(tokens);
	}
	return envpp;
}

char ** process_main(char input[ARG_LIMIT], char* argv[], char* envpp[]) {
	if (argv[1] != NULL) {
		envpp = cmd_line_arg_case(input, argv, envpp);
	} else {
		envpp = interactive_case(input, envpp);
	}
	return envpp;
}

//Support changing current directory ( cd ) -
//Execute binaries interactively
//Execute scripts
//Execute pipelines of binaries ( /bin/ls | /bin/grep test )
//Set and use PATH and PS1 variables

//todo : whitespave checks
int main(int argc, char* argv[], char* envpp[]) {
	char input[ARG_LIMIT];
	envpp = process_main(input, argv, envpp);
	return 0;
}
