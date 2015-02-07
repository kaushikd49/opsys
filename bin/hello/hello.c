#include<stdio.h>
#include<syscall.h>
#include<../libc/stdlib.c>
int main(int argc, char* argv[], char* envp[]) {

	pid_t pid = fork();
	if(pid>=0){
		if(pid==0){
			char *msg = "inside child bro\n";
			size_t length = strlen(msg);
			write(1,msg,length);
			exit(0);
		}
		else{
			int status;
			//uint64_t hello = 12;

			waitpid(-1, &status, 0);
			int i =1024244;
			char *str = "hello child";
			printf("inside parent(s) %d process %s\n",i,str);
			exit(0);
		}
	}
	else{
		char msg2[] = "unsuccessful fork bro\n";
		size_t length = strlen(msg2);
		write(1,msg2,length);
	}
	exit(0);

}
