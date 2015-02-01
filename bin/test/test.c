#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define NULL ((void *)0)
char *getEnv(char *substr, char *str[]) {
	size_t length = strlen(substr);
	int i = 0;
	while (str[i] != NULL) {
		if (strncmp(substr, str[i], length) == 0)
			return str[i] + length;
		i++;
	}
	return "error";
}
void setPS1(char *envp[], char *newPrompt) {
	char ps1[] = "PS1=";
	int i = 0;
	size_t length = strlen(ps1);
	while (envp[i] != NULL) {
		if (strncmp(ps1, envp[i], length) == 0)
			break;
		i++;
	}
	if (envp[i] != NULL) {
		strcat(ps1, newPrompt);
		envp[i] = (char *) malloc(sizeof(ps1));
		strcpy(envp[i], ps1);
	}
	printf("%s in function %d\n", envp[i], strlen(newPrompt));
}
int isinEnv(char *newStr, char *envp[]) {
	int indexequal = 0;
	while (newStr[indexequal] != '\0') {
		if (newStr[indexequal] == '=')
			break;
		indexequal++;
	}
	if (newStr[indexequal] == '\0') {
		return -1;
	}
	int current = 0;
	while (envp[current] != NULL) {
		if (strncmp(newStr, envp[current], indexequal + 1))
			return 1;
		current++;
	}
	return 0;
}
void printEnviron(char *envp[]) {
	int current = 0;
	while (envp[current] != NULL) {
		if (current > 23)
			printf("\n%s\n", envp[current]);
		current++;
	}
}
int main(int argc, char *argv[], char *envp[]) {
	int status, i = 0, j = 0;
	pid_t child;

	char buffer[200];
	//char **envpp = envp;
	//int death;
	//int a = sizeof(i);

	char ps1[] = "PS1=prompt>>";

	while (envp[i] != NULL) {
		i++;
	}
	char *dupenvp[i + 2];
	while (j != i) {
		dupenvp[j] = envp[j];
		j++;
	}
	dupenvp[j] = (char *) ps1;
	dupenvp[j + 1] = NULL;
	i = 0;
	//*envpp = (char *)dupenvp;

	while (1) {
		char *prompt = getEnv("PS1=", dupenvp);
		printf("%s", prompt);
		scanf("%s", buffer);
		if (strncmp("PS(", buffer, 3) == 0) {
			//printf("%s",buffer+3);

			setPS1(dupenvp, buffer + 3);
			printEnviron(dupenvp);
		} else if (strncmp("exit", buffer, 4) == 0) {
			printf("goodbye..");
			break;
		} else {
			child = fork();
			if (child >= 0) {
				if (child == 0) {
					//printf("child process\n");
					execve(buffer, NULL, NULL);
					return 0;
					//printf("error\n");
				} else {
					//printf("parent process started\n");
					waitpid(-1, &status, 0);
					//printf("child with ID %d done\n",death);
					//printf("parent done\n");
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
