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

//int main(int argc, char* argv[], char* envp[]) {
//
//	char input[ARG_LIMIT];
//	while (1) {
//		printf("shellzhere>> ");
//		scanf(" %1000[^\n]", input);
////		printf("input is %s ", input);
//		if (strcmp(input, "exit") == 0) {
//			break;
//		}
//		char ** tokens = advance_tokenize(input, ' ', '"');
//		take_action(tokens);
//		free(tokens);
//	}
//
//	return 0;
//}


int main(int argc, char *argv[], char *envp[]) {
	int status;
	pid_t child;
	char buffer[200];
	char ps1[] = "PS1=prompt>>";
	char **envpp = envp;
	envpp = setEnv(ps1, envpp);
	while (1) {
		char *prompt = getEnv("PS1=", envpp);
		printf("%s", prompt);
		scanf("%s", buffer);
		if (strncmp("PS(", buffer, 3) == 0) {
			const char*tempbuffer = buffer + 3;
			char temp[205] = "PS1=";
			strcat(temp, tempbuffer);
			envpp = setEnv(temp, envpp);
		} else if (strncmp("exit", buffer, 4) == 0) {
			break;
		} else if (strncmp("setenv", buffer, 6) == 0) {
			int size = strlen("PS1") + strlen("helloworld>>") + strlen("=") + 1;
			char *newStr = (char *) malloc(size * sizeof(char));
			strcpy(newStr, "PS1");
			strcat(newStr, "=");
			strcat(newStr, "helloworld>>");
			envpp = setEnv(newStr, envpp);
		} else if (strncmp("getenv", buffer, 6) == 0) {
			char *getenvOutput = getEnv("PS1", envpp);
			if (getenvOutput != NULL) {
				printf("%s\n", getenvOutput + 1);
			} else
				printf("\n");
		} else {
			child = fork();
			if (child >= 0) {
				if (child == 0) {

					char pathPrefix[250];
					char *current, *next;
					char *newargs[] = { "echo", "$PATH", NULL }; // this is what Input has to change.
					execve(newargs[0], newargs, NULL);
					char *path = getEnv("PATH=", envpp);
					current = path;
					//printf("%s",path);
					while (1) {
						next = strchr(current, ':');
						if (next != NULL) {
							strcpybtwptrs(current, next, pathPrefix);
							strcat(pathPrefix, "/");
							strcat(pathPrefix, newargs[0]);
							//printf("%s\n",pathPrefix); //debug printf
							execve(pathPrefix, newargs, NULL);
							current = next + 1;
						} else {
							printf("command not found\n");
							break;
						}
					}

					//printf("error\n");
					return 0;
				} else {
					waitpid(-1, &status, 0);
				}
			}

			else {
				printf("internal error unsuccessful fork");
				exit(0);
			}
		}
	}
	return 0;

}
