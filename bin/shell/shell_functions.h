#include<stdio.h>
#include<stdlib.h>
//#include<string.h>
//#define NULL ((void *)0)

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
//void free_char_array()
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

void print_tokens(char ** tokens) {
	int i = 0;
	printf("received tokens: ");
	while (tokens[i] != NULL) {
		printf("%d %s ", (i + 1), tokens[i]);
		i++;
	}
	printf("\n");
}
void free_char_array(char **tokens){
	if(tokens == NULL){
		return;
	}
	char **current = tokens;
	while(*current !=NULL){
		free(*current);
		current =current+1;
	}
	free(tokens);
}
char **advance_tokenize(char *input, char delim, char fieldEncloser) {
	int numDelims = num_delims(input, delim);
	printf("NUMDELIMS:%d\n",numDelims);
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
		printf("\n%s\n", envp[current]);
		current++;
	}
}
char **setEnvStack(char *newVar, char *envpp[]) {
	if (isinEnv(newVar, envpp)) {
		int index = getEnvIndex(newVar, envpp);
		//printf("update variable %s\n", newVar);
		if (index != -1)
			envpp[index] = newVar;
		return envpp;
	} else {
		//printf("Initialize variable %s\n", newVar);
		int i = 0, j = 0;
		while (envpp[i] != NULL)
			i++;
		size_t newSize = sizeof(char *) * (i + 2);
		char **dupenvp = (char **) malloc(newSize);
		while (j != i) {
			dupenvp[j] = (char *) envpp[j];
			j++;
		}
		dupenvp[j] = (char *) newVar;
		dupenvp[j + 1] = NULL;
		//free(envpp);  //since it is on stack we can't free it.
		return dupenvp;
	}
	return envpp;
}
char **setEnv(char *newVar, char *envpp[]) {
	if (isinEnv(newVar, envpp)) {
		int index = getEnvIndex(newVar, envpp);
		//printf("update variable %s\n", newVar);
		if (index != -1)
			envpp[index] = newVar;
		return envpp;
	} else {
		//printf("Initialize variable %s\n", newVar);
		int i = 0, j = 0;
		while (envpp[i] != NULL)
			i++;
		size_t newSize = sizeof(char *) * (i + 2);
		char **dupenvp = (char **) malloc(newSize);
		if(dupenvp == NULL){
			int backupErrno = errno;
			errorHandler(backupErrno);
			return envpp;
		}
		while (j != i) {
			dupenvp[j] = (char *) envpp[j];
			j++;
		}
		dupenvp[j] = (char *) newVar;
		dupenvp[j + 1] = NULL;
		free(envpp);
		return dupenvp;
	}
	return envpp;
}
/*
 * tested for no extra line in the end,
 *  empty file,no whitespace in last line
 *
 */
int read_line(char *buffer, int FD) {
	int i = 0, flag = 1, fileRet;
	fileRet = read(FD, buffer + i, 1);
	while (fileRet != 0 && *(buffer + i) != '\n') {
		i++;
		fileRet = read(FD, buffer + i, 1);
	}
	if (fileRet == 0)
		flag = 0;
	*(buffer + i) = '\0';
	return flag;
}
void errorHandler(int errorCode){

	switch(errorCode){
		case EBADF:
			printf("%d: Bad file  \n",EBADF);
			break;
		case READERR:
			printf("%d:General Read Error\n", READERR);
			break;
		case WRITEERR:
			printf("%d:General Write Error\n", WRITEERR);
			break;
		//open errors
		case ENOMEM:
			printf("\n%d: Out of memory or can't allocate memory of that size \n", ENOMEM);
			break;
		case EACCES:
			printf("\n%d: Not enough Permissions\n", EACCES);
			break;
		case OPENERROR:
			printf("\n%d: General Open Error \n", OPENERROR);
			break;
		case ENOENT:
			printf("\n%d: This file or directory does not exist \n", ENOENT);
			break;
		case EEXIST:
			printf("\n%d: File Exists\n", EEXIST);
			break;
		case EDQUOT:
			printf("\n%d: Maximum number of files opened by process \n", EDQUOT);
			break;
		case EFAULT:
			printf("\n%d: Illegal memory access \n", EACCES);
			break;
		case EFBIG:
			printf("\n%d: File too big\n", EFBIG);
			break;
		case EINTR:
			printf("\n%d: Interrupt occur \n", EINTR);
			break;
		case EISDIR:
			printf("\n%d: the path is an address \n", EISDIR);
			break;
		case ELOOP:
			printf("\n%d: too many symbolic jumps \n", ELOOP);
			break;
		case EMFILE:
		printf("\n%d: Too many files open\n", EMFILE);
			break;
		case ENAMETOOLONG:
			printf("\n%d: The name of argument is too long\n", ENAMETOOLONG);
			break;
		case ENFILE:
			printf("%d: Too many files \n", ENFILE);
			break;
		case ENODEV:
			printf("%d: Device does not exist\n", ENODEV);
			break;
		case ENOSPC:
			printf("%d: No memory left \n", ENOSPC);
			break;
		case ENOTDIR:
			printf("%d: path not a directory\n", ENOTDIR);
			break;
		case ENXIO:
			printf("\n%d: The device or memory address does not exist\n", ENXIO);
			break;
		case EOVERFLOW:
			printf("%d: overflow error \n",EOVERFLOW);
			break;
		case EPERM:
			printf("\n%d: Operation does not have the required permission\n",EPERM);
			break;
		case EROFS:
			printf("%d: no write/execute permission \n", EROFS);
			break;
		case ETXTBSY:
			printf("%d: operation being performed on file \n", ETXTBSY);
			break;
		//end open errors
		//fork errors
		case EAGAIN:
			printf("\n%d: Try Again, currently blocked \n", EAGAIN);
			break;
		//execve errors
		case E2BIG:
			printf("\n%d: Too many Args \n", E2BIG);
			break;
		case ENOEXEC:
			printf("\n%d: Not an Executable\n", ENOEXEC);
			break;
		case EIO:
			printf("\n%d: I/O error\n", EIO);
			break;
		case EXECVEERROR:
		//wait pid errors
		case EINVAL:
			printf("%d: Invalid descriptor/id \n", EINVAL);
			break;
		case ECHILD:
			printf("\n%d: Child process does not exist \n",ECHILD );
			break;
		//lseek
		case LSEEKERROR:
			printf("%d: General error in LSEEK  \n", LSEEKERROR);
			break;
		//sleep
		case NANOSLEEPERROR:
			printf("%d: General error in NANOSLEEPERROR\n", NANOSLEEPERROR);
			break;
		case CHDIRERROR:
			printf("%d: Error with change directory\n", CHDIRERROR);
			break;
		default:
			printf("Error occured with error code:%d\n",errorCode);
			break;
	}
}
