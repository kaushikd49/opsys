#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define NULL ((void *)0)
int main(int argc, char *argv[], char *envp[]){
	int status;	
	pid_t child;
	//int death;
	char buffer[200];
	while(1){
		printf(">>");
		scanf("%s",buffer);
		child=fork();	
		if(child>=0){
			if(child==0){
				//printf("child process\n");
				execve(buffer,NULL,NULL);
				//printf("error\n");
			}
			else {
				//printf("parent process started\n");
				waitpid(-1,&status,0);
				//printf("child with ID %d done\n",death);
				//printf("parent done\n");	
			}
		}
		else{
			printf("internal error unsuccessful fork");
			exit(0);
		}
	}
	return 0;

}
