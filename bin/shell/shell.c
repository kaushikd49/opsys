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
