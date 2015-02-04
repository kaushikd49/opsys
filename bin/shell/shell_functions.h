#include<stdio.h>
#include<stdlib.h>
//#include<string.h>
#define NULL ((void *)0)




// Returns num of delims just based on their presence.
// Not escaping aware, which is fine for now as this method
// is used only to find the size of char ** tokens[] and the
// value returned will always be >= num of tokens actually
// generated.
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
		// First char itself is a possible token's start location
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

char * escape_aware_copy(char *input, int startIdx, int endIdx, char delim) {
	char *token = (char*) malloc((endIdx - startIdx) * sizeof(char));
	int i = startIdx, j = 0;
	while (i < endIdx) {
		if (input[i] == '\\') {
			if ((i + 1) < endIdx && input[i + 1] == delim) {
				// escaped delim copy
				token[j++] = input[i + 1];
				i += 2;
				continue;
			} else {
				token[j++] = input[i];
			}
		} else {
			token[j++] = input[i];
		}
		i++;
	}
	return token;
}

void captureToken(char **tokens, char *input, int *endIdx, int *startIdx,
		char delim, int i, int *tokenIdx) {
	*endIdx = i;
	char * token = escape_aware_copy(input, *startIdx, *endIdx, delim);
	tokens[*tokenIdx] = token;
	*tokenIdx = *tokenIdx + 1;
	*startIdx = -1;
	*endIdx = -1;
}

void take_action(char ** tokens) {
	int i = 0;
	printf("received tokens\n");
	while (tokens[i] != NULL) {
		printf("%s\n", tokens[i]);
		i++;
	}
}

char **advance_tokenize(char *input, char delim, char fieldEncloser) {
	int numDelims = num_delims(input, delim);
	char **tokens = (char**) malloc((numDelims + 2) * sizeof(char*));
	int i = 0, j = 0, startIdx = -1, endIdx = -1, fieldEnclStarted = 0;
	while (input[i] != '\0') {
		if (input[i] != delim) {
			if (input[i] == '\\' && input[i + 1] == fieldEncloser) {
				// handle backslash quote (\")
				if (startIdx == -1) {
					startIdx = i + 1;
				}
				i += 2;
				continue;
			} else if (input[i] != fieldEncloser && startIdx == -1) {
				startIdx = i;
			} else if (input[i] == fieldEncloser) {
				if (startIdx == -1) {
					// token opens with fieldEncloser
					startIdx = i + 1;
					fieldEnclStarted = 1;
				} else {
					// token opens with fieldEncloser
					captureToken(tokens, input, &endIdx, &startIdx, delim, i,
							&j);
					fieldEnclStarted = 0;
				}
			}
		} else if (fieldEnclStarted != 1 && startIdx > -1) {
			captureToken(tokens, input, &endIdx, &startIdx, delim, i, &j);
		}
		i++;
	}
	if (startIdx > -1)
		captureToken(tokens, input, &endIdx, &startIdx, delim, i, &j);
	tokens[j] = NULL; 	//sentinel
	return tokens;
}



// Imported below from Muthukumar's test.c

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


