//#include "../shell/shell.c"
#include<stdio.h>
#include<stdlib.h>
#include<syscall.h>
#include<string.h>
#include<../bin/shell/shell_functions.h>
//#include<signal.h>
#include<errno.h>
/*
struct timespec{
	time_t tv_sec;
	long tv_nsec;

};*/
/*
int main(int argc, char *argv[], char *envp[]) {
//	printf("%d", dup2(newfd, 0));

//	int newfd = open("/tmp/file.txt", O_WRONLY);
//	do_execute(tokens, envpp);

	static struct timespec rqtp;
	static struct timespec tmqp;
	rqtp.tv_sec = 5;
	rqtp.tv_nsec = 0;
	static struct timespec rqtp1;
    static struct timespec tmqp1;
		rqtp1.tv_sec = 2;
		rqtp1.tv_nsec = 0;
	int pid = fork();
	if(pid>=0){
		if(pid > 0){
			printf("child");
						nanosleep(&rqtp1,&tmqp1);

						syscall_2_test(62, pid,2);
						if(errno == EINTR)
							printf("interupt");
						printf("sup%d", tmqp.tv_sec);
						exit(0);


		}
		else{
			printf("a");
			nanosleep(&rqtp,&tmqp);

			printf("sup%d", tmqp.tv_sec);
			printf("b");
			exit(0);

		}
	}
	else{
		exit(0);
	}
*//*
	int fildes[2];

	char buf[100];
	int nbytes;
	int status = pipe(fildes);
	if(status == -1)
		printf("error status");
	printf("status%d",status );
	int pid = fork();
	if(pid>=0){
		if(pid == 0){
			close(fildes[1]);
			nbytes = read(fildes[0],buf,100);
			printf("nbytes%d buffer: %s",nbytes, buf);
			close(fildes[0]);
			exit(0);
			//printf("%d",sleep(10));
		}
		else{
			//printf("hello");
			//int child_status;
			//waitpid(-1, &child_status, 0);
			//sleep(2);
			//syscall_2_test(62, pid, 18);
			//syscall_2_test(62, pid, 25);
			close(fildes[0]);
			printf("\nparent");
			int val = sleep(10);
			printf("\n parent sleep end");
			char val1[2];
			val1[0]= (char)(48+val);
			val1[1]= '\0';
			//char *val2 = &val1;
			write(fildes[1],val1,2);
			close(fildes[1]);
			exit(0);
		}
	}
	else{
		exit(0);
	}
	return 0;
}
*/

//int main(int argc,char *argv[], char *envp[] )
//{
//	int pid = fork();
//	if(pid>0){
//		if(pid == 0){
//			sleep(10);
//			exit(0);
//		}
//		else{
//
//			alarm(5);
//		}
//	}*/
//	alarm(5);
//	printf("%d",sleep(10));
//	sleep(6);
//	while(1);
//
//	char *str = (char *)423423;
//	int size = 1000;
//	//char *buf = str;
//	char *ptr = getcwd(str,size);
//	if(ptr !=NULL)
//		printf("%s", ptr);
//	else{
//		printf("%d",errno);
//	}
//	return 0;
//}

int main(int argc, char *argv[], char *envp[]){
//	char str[10] = "fasf";
//	//int size = 1000;
//	int readans = open(str , O_RDONLY);
//	if(readans == -1){
//	errorHandler(errno);
//	}
//	printf("asdasd:%d\n", readans);
//	int result = brk((void *)54);
//	if(result == -1){
//		errorHandler(errno);
//	}
//	int *result1 = (int *)malloc(1000000000*sizeof(int));
//	if(result1 == NULL){
//		errorHandler(errno);
//	}
	void *returnval = opendir("./bin/");
	if(returnval ==NULL){
		errorHandler(errno);
		exit(0);
	}
	struct dirent *ans =NULL;
	while((ans = readdir(returnval))!=NULL ){
		printf("%s\n",ans->d_name);
	}
	printf("%d", closedir(returnval));
}
