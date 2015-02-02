#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define NULL ((void *)0)
 char *getEnv(char *substr, char *str[]){ //function returns the string of the environment variable if it is present or 
	size_t length = strlen(substr);		  //returns the string error
	int i =0;							  //this function would be easy to break(minly optimized for PS1), we should make it 
	while(str[i]!=NULL){				  //better later.
		if(strncmp(substr,str[i],length) == 0)
			return str[i]+length; 
		i++;
	}
	return "error";
}
void setPS1(char *envp[], char *newPrompt){//function to setPS1, we should look at writing a standard method to set any 
	char ps1[] = "PS1=";					//environment variable.
	int i=0;
	size_t length = strlen(ps1);
	while(envp[i]!=NULL){
		if(strncmp(ps1,envp[i],length) == 0)
			break;
		i++;
	}
	if(envp[i]!=NULL){
		envp[i] = newPrompt;
	}
}
int isinEnv(char *newStr, char *envp[]){  //not using yet returns 1,0,-1 if the env variable is present, not present, error env variable
	int indexequal = 0;
	while(newStr[indexequal]!='\0'){
		if(newStr[indexequal]=='=')
			break;
		indexequal++;
	}
	if(newStr[indexequal]=='\0'){
		return -1;
	}
	int current = 0;
	while(envp[current] !=NULL){
		if(strncmp(newStr, envp[current],indexequal+1))
			return 1;
		current++;
	}
	return 0;
}
void printEnviron(char *envp[]){  //debug function to show the last few lines of environment variables.
	int current = 0;
	while(envp[current]!=NULL){
		if(current>23)
			printf("\n%s\n", envp[current]);
		current++;
	}
}
int main(int argc, char *argv[], char *envp[]){
	int status,i=0,j=0;
	pid_t child;
	char buffer[200];
	char ps1[] = "PS1=prompt>>";
	while(envp[i]!=NULL){
		i++;
	}
	char *dupenvp[i+2];
	while(j!=i){
		dupenvp[j] = envp[j];
		j++;
	}
	dupenvp[j] = (char *)ps1;
	dupenvp[j+1] = NULL;
	i=0;
	while(1){
		char *prompt = getEnv("PS1=", dupenvp);
		printf("%s", prompt);
		scanf("%s",buffer);
		if(strncmp("PS(",buffer,3)==0){
			const  char*tempbuffer = buffer+3;
			char temp[205] = "PS1=";
			strcat(temp,tempbuffer);
			setPS1(dupenvp,temp);
		}
		else if(strncmp("exit", buffer,4)==0){
			break;
		}
		else{
		child=fork();
		if(child>=0){
			if(child==0){
				
				char pathPrefix[250];
				char *current, *next;
				char *newargs[] = {"bs","-l",NULL}; // this is what Input has to change.
				execve(newargs[0],newargs,NULL);
				char *path = getEnv("PATH=",dupenvp);
				current = path;
				//printf("%s",path);
				while(1){
					next =strchr(current, ':'); 
					if(next!=NULL){
						strcpybtwptrs(current,next,pathPrefix);
						strcat(pathPrefix,"/");
						strcat(pathPrefix, newargs[0]);
						//printf("%s\n",pathPrefix); //debug printf
						execve(pathPrefix,newargs,NULL);
						current=next+1;
					}
					else{
						printf("command not found\n");
						break;
					}
				}
				
				//printf("error\n");
				return 0;
			}
			else {
				waitpid(-1,&status,0);	
			}
		}		
		
		else{
			printf("internal error unsuccessful fork");
			exit(0);
		}
		}
	}
	return 0;

}
