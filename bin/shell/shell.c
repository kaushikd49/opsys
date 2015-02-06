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
					exit(0);
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
	if(cmd == NULL){
		printEnviron(envpp);
		return;
	}
	if(tokens[2]!=NULL){
		printf("too many arguments to getenv\n");
		return;
	}
	char* getenvOutput = getEnv(cmd, envpp);
	if(getenvOutput[0] != '='){
		printf("\n");
		return;
	}
	if (getenvOutput != NULL)
		printf("%s\n", getenvOutput + 1);
	else
		printf("\n");
}
/*
 * setenv: TESTED
 * more checks done:
 * if first only setenv is the command prints all the envvars
 * if one arg then the env variable is set to blank
 * if 2 deals with normally
 * solved the seg fault problem
 * Similar for getenv
 * NOT KNOWN: why does top= come in envpp, just run setenv <no arguments> and see!!
 */
char ** handleSetenv(char **tokens, char *envpp[]) {
	int envStrlen = 0,cmdlen = 0;
	char *envStr = NULL, *cmd = NULL;
	if(tokens[1]!=NULL){
		cmd = tokens[1];
		cmdlen = strlen(cmd);
	}
	else{
		printEnviron(envpp);
		return envpp;
	}

	if(tokens[2]!=NULL){
		envStr = tokens[2];
		envStrlen = strlen(envStr);
	}
	int size = cmdlen + envStrlen + 2;
	char* newStr = (char*) malloc(size * sizeof(char));
	strcpy(newStr, cmd);
	strcat(newStr, "=");
	if(envStr !=NULL)
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
/*
 * tested file handling when no file as such
 * when I pass multiple files, only first script is run
 * not checking comments in between coz its not needed.
 * NOT TESTED: proper scripts with pipe etc,
 * NOT TESTED: the individual functions it calls, check
 * function def for details of those tests.
 */
char** cmd_line_arg_case(char input[ARG_LIMIT], char* argv[], char* envpp[]) {
	int flag = 1;
	//todo: check for size greater than ARG_LIMIT
	int fileHandle = open(argv[1], O_RDONLY);
	if (fileHandle < 0){
		printf("Opening Script: Failed, No such Script exists\n");
		return envpp;
	}
	do {
		flag = read_line(input, fileHandle);
		char** tokens = advance_tokenize(input, ' ', '"');
		if (tokens[0] == NULL || strncmp(tokens[0], "#", 1) == 0)
			continue;
		envpp = take_action(tokens, envpp);
	} while (flag == 1);
	close(fileHandle);
	
	return envpp;
}
/*
 * FIXED BUG: setenv without arguments setenv with 1 argument.
 *
 */
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

char ** process_main(int argc, char* argv[], char* envpp[]) {
	char input[ARG_LIMIT];
	if (argc >= 2) {   //note it will run only the first script if it has more than one script as parameter. like normal shell
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
	envpp = process_main(argc, argv, envpp); //made input inside the process_main
	return 0;								//and checking if it is script with 1 script only
}
