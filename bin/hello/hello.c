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
			//void *ptr = NULL;
			int *ptr = (int *)malloc(sizeof(int)*10);
			char *ptr1 = malloc(sizeof(char)*4);
			for(i=0;i<10;i++)
				ptr[i]=i;
			*ptr1 ='a';
			*(ptr1+1) = 'b';
			*(ptr1+2) = 'c';
			*(ptr1+3) = '\0';
			/*
			for(i=0;i<4;i++)
				printf("%d\t",ptr[i]);
			printf("\n%s", ptr1);
			*/
			free(ptr);
			int *ptr3 = (int *)malloc(sizeof(int)*5);
			ptr3[0] = 33;
			/*
			printf("\nptr3:%d\n",ptr3[0]);
			*/
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
