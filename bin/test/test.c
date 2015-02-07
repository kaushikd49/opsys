#include<stdio.h>
#include<stdlib.h>
#include<string.h>
//#define NULL ((void *)0)
char *getEnv(char *substr, char *str[]) { //function returns the string of the environment variable if it is present or
	size_t length = strlen(substr);		  //returns the string error
	int i = 0;//this function would be easy to break(minly optimized for PS1), we should make it
	while (str[i] != NULL) {				  //better later.
		if (strncmp(substr, str[i], length) == 0)
			return str[i] + length;
		i++;
	}
	return NULL;
}
//purpose of this is usually for changing the path variable.
int getEnvIndex(char *substr, char *str[]) {
	int i = 0, indexequal = 0;
	while (substr[indexequal] != '\0' && substr[indexequal] != '=') {
		indexequal++;
	}
	while (str[i] != NULL) {
		if (strncmp(substr, str[i], indexequal + 1) == 0) {
			return i;
		}
		i++;
	}
	return -1;
}
void setPS1(char *envp[], char *newPrompt) {//function to setPS1, we should look at writing a standard method to set any
	char ps1[] = "PS1=";					//environment variable.
	int i = 0;
	size_t length = strlen(ps1);
	while (envp[i] != NULL) {
		if (strncmp(ps1, envp[i], length) == 0)
			break;
		i++;
	}
	if (envp[i] != NULL) {
		envp[i] = newPrompt;
	}
}
int isinEnv(char *newStr, char *envp[]) { //not using yet returns 1,0,-1 if the env variable is present, not present, error env variable
	int indexequal = 0;
	while (newStr[indexequal] != '\0' && newStr[indexequal] != '=') {
		indexequal++;
	}
	int current = 0;
	while (envp[current] != NULL) {
		if (strncmp(newStr, envp[current], indexequal + 1) == 0) {
			return 1;

		}
		current++;
	}
	return 0;
}
void printEnviron(char *envp[]) { //debug function to show the last few lines of environment variables.
	int current = 0;
	while (envp[current] != NULL) {
		if (current > 23)
			printf("\n%s\n", envp[current]);
		current++;
	}
}
char **setEnv(char *newVar, char *envpp[]) {
	if (isinEnv(newVar, envpp)) {
		int index = getEnvIndex(newVar, envpp);
		printf("update variable %s\n", newVar);
		if (index != -1)
			envpp[index] = newVar;
		return envpp;
	} else {
		printf("Initialize variable %s\n", newVar);
		int i = 0, j = 0;
		while (envpp[i] != NULL)
			i++;
		size_t newSize = sizeof(char *) * i + 2;
		char **dupenvp = (char **) malloc(newSize);
		while (j != i) {
			dupenvp[j] = (char *) envpp[j];
			j++;
		}
		dupenvp[j] = (char *) newVar;
		dupenvp[j + 1] = NULL;
		//printf("hello");
		//printEnviron(dupenvp);
		return dupenvp;

	}
}
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
