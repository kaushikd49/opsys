//#include<stdio.h>
//#include<syscall.h>
//#include<stdlib.h>
////#include<errno.h>
////#include<../libc/stdlib.c>
////extern __thread int errno;
//void child_sample_exec(char* argv[], char* envp[]) {
//	printf("**child now executing ls with chdir**\n");
//	chdir("/");
//	errno = 0;
//	int retVal = execve("/bin/ls", argv, envp);
//	if (retVal == -1) {
//		printf("hello:%d", errno);
//	}
//}
//
//void parent_sample_exec(char* argv[], char* envp[]) {
//	printf("**parent now executing date**\n");
//	execve("/bin/date", argv, envp);
//}
//
//void sample_file_readwrite() {
//	char *file = "/tmp/temp.txt";
//	char *buffer = "we the people of the world";
//	printf("attempting to write into %s\n", file);
//	// todo: find a way to remove hard-coding of permission below
//	int write_fd = open(file, 2);
//	write(write_fd, buffer, 10);
//	close(write_fd);
//}
//
//int main(int argc, char* argv[], char* envp[]) {
//
////	int newfd = dup(0);
////	printf("%d", newfd);
////	char buf[10]="abcdefg\n";
////	write(newfd, buf, 6);
////	close(newfd);
////	exit(0);
//
////	int a = 12;
////	scanf("%d", &a);
////	printf("%d", a);
////
//
//	while (1) {
//		char s[22] = "";
//
//		int x = -2;
//		int a = 0;
//		int b;
//// with duplicate type specifiers crap happens : todo
//		int sc = scanf("%s %d %x %c", s, &x, &a, &b);
//		int pf = printf("1) %s 2) %d 3) %x 4) %c", s, x, a, b);
//		printf("\n%d %d\n", sc, pf);
//	}
//
////	printf("%s", s);
////	printf("%d",x);
////	printf("%x",a);
////	printf("%c",b);
//
////	int a;
////	scanf("%x", &a);
////	printf("%d", a);
////
////	char c ;
////	scanf("%c", &c);
////	printf("%c", c);
//
////	int a = 22;
////	scanf("%d", &a);
////	printf("%x", a);
//
//	pid_t pid = fork();
//	if (pid >= 0) {
//		if (pid == 0) {
//			char *msg = "inside child bro.\n";
//			size_t length = strlen(msg);
//			write(1, msg, length);
//			printf("parent must be %d   %d\n", getppid(), getpid());
//			int ret = chdir("./script1.sh");
//			printf("CHDIRRET: %d  %d", ret, errno);
//			child_sample_exec(argv, envp);
//
//			exit(0);
//		} else {
//			int status;
//			//uint64_t hello = 12;
//
//			int ret = waitpid(-1, &status, 0);
//			printf("\nwait:%d\n", ret);
//			printf("\nERRNO:%d", errno);
//
//			//char *str = "hello child";
//			//printf("\ninside parent process %d. %s\n", getpid(), str);
//			//void *ptr = NULL;
//			//int *ptr = (int *) malloc(sizeof(int) * 10);
//			//char *ptr1 = malloc(sizeof(char) * 4);
//			//int i = 1024244;
//			//for (i = 0; i < 10; i++)
//			//	ptr[i] = i;
//			//*ptr1 = 'a';
//			//*(ptr1 + 1) = 'b';
//			//*(ptr1 + 2) = 'c';
//			//*(ptr1 + 3) = '\0';
//			/*
//			 for(i=0;i<4;i++)
//			 printf("%d\t",ptr[i]);
//			 printf("\n%s", ptr1);
//			 */
//			//free(ptr);
////			int *ptr3 = (int *) malloc(sizeof(int) * 10);
////			if(ptr3 ==NULL)
////				exit(1);
////			ptr3[0] = 33;
////			printf("\nptr3:%d\n",ptr3[0]);
//			//sample_file_readwrite();
////			parent_sample_exec(argv, envp);
//			//printf("\n");
//			//testing strcpy
//			//char *ptr4 = (char *)malloc(sizeof(char) * 10);
//			//char str4[10] = "S";
//			//strcpy(ptr4,str4);
//			//printf("\n%s\n", ptr4);
//			//testing strncmp(done all possible tests)
//			//char *ptr5 = "";
//			//char *ptr6 = "ABC";
//			//printf("%s",ptr5);
//			//printf("ANSWER:%d", strncmp(ptr5,ptr6,3));
//			//testing strchr
//			//printf("\nSTRCHR:%s",strchr(ptr6,'A'));//dont try a character not there seg fault but thats supposed to happen
//			//testing strcat
//			//printf("\nSTRCAT:%s", strcat(ptr4,ptr5));
//			//int fd = open("./fdf",0);
//			//printf("fff:%d",(signed long)fd);
//			//int i = -13;
//			//printf("%d",i);
//			//char buffer[50] = "helloada";
//			//write(fd, buffer, 50);
//			/*
//			 char fileName[100] = "./";
//			 DIR *dirStr = opendir(fileName);
//			 struct dirent *temp;
//			 while((temp = readdir(dirStr))!=NULL){
//			 printf("\nDENTRY:%s  %d",temp->d_name,temp->d_reclen);
//			 }
//			 closedir(dirStr);*/
//			//
//			//
//			//
//			//
//			//
//			//
//			//
//			//
//			//
//			//1)exit no test done
//			//2)read
//			/*
//			 int fd[10000],i = 0;
//			 while(i!=1){
//			 errno = 0;
//			 fd[i] = open("./corrupt.txt",O_WRONLY);
//			 printf("\n%d  %d",i,errno);
//			 i++;
//			 //scanf("%d",ch);
//			 }*/
//			/*
//			 char buffer[5];
//			 errno = 0;
//			 int ret = read(fd[0],buffer,50);
//
//			 printf("\n%d",ret);
//			 */
//			//		int result
//			//lseek
//			int fd = open("test.txt", O_RDONLY);
//			off_t offset = lseek(fd, 5, 6);
//			if ((int64_t) offset == -1)
//				printf("%d", errno);
//			//offset = lseek(fd,-3,SEEK_END);
//			//printf("\n");
//			//printf("%d",offset);
//			//char str[5];
//			//read(fd,str,2);
//			//printf("%s",str);
//			exit(0);
//		}
//	} else {
//		char msg2[] = "unsuccessful fork bro\n";
//		size_t length = strlen(msg2);
//		write(1, msg2, length);
//	}
//	exit(0);
//
//}
#include <stdlib.h>
#include <stdarg.h>
#include <sys/defs.h>
#include <sys/syscall.h>
#include <stdio.h>
#include<stdlib.h>
int arr[100];
int b = 10;
extern __thread int errno;
int foo(int a) {
	return 1;
}
int main(int argc, char* argv[], char* envp[]) {
	int c = 4;
	foo(b);
	getpid();
	pid_t ret = fork();
	c += 21;
	b = b + 35;
	if (ret == 0) {
		printf("\nCHILD ret:b:c %d:%d:%d, pid:%d parent:%d\n", ret, b, c,
				getpid(), getppid());
		pid_t ret2 = fork();
		if (ret2 == 0) {
			printf("\nNESTED CHILD %d \n", ret2);
		} else if (ret2 > 0) {
			printf("\nNESTED PARENT %d \n", ret2);
		} else if (ret2 < 0) {
			printf("\nERROR \n", ret2);
		}
		execve("bin/hel", argv, envp);
		exit(0);
	} else if (ret > 0) {
		int status;
		waitpid(-1, &status, 0);
		printf("\nPARENT ret:b:c %d:%d:%d, pid:%d parent:%d\n", ret, b, c,
				getpid(), getppid());
	} else {
		printf("\nHello! ERROR ret:b:c %d:%d:%d, pid:%d parent:%d\n", ret, b, c,
				getpid(), getppid());
	}

//	b =5;
//	foo(b);
	//	if(val == 0){
//		printf("\nSon, pid:%d fork_ret:%d parent:%d\n", getpid(), val, getppid());
//	}else if(val>0){
//		printf("\nDad, pid:%d fork_ret:%d parent:%d\n", getpid(), val, getppid());
//	}else {
//		printf("\nprocess:%d fork error, returned %d", getpid(),val);
//	}

//	b = 2;
//	b = 1;
//	fork();
//	printf("Hello World!%d %s %s \n", argc, argv[0], envp[0]);
//	pid_t val = fork();
//	printf("Hello World!. Getpid %d Ret val is %d\n", getpid(), val);
//	int fd = open("test/test.txt", 0);
//	char buff[10];
//	for(int64_t i = 0; i<10; i++){
//		buff[i] = 0;
//	int fd = open("test/test.txt", 0);
//	char buff[10];
//	for(int64_t i = 0; i<10; i++){
//		buff[i] = 0;
//	}
//	read(fd, buff, 5);
//	printf("write:::%s", buff);
//	printf("g\n");
//	read(fd, buff, 7);
//	printf("h\n");
//	printf("write2:::%s", buff);
//	int *a = (int *)malloc(5*sizeof(int));
//	a[0] = 2;
//	a[1] = 4;
//	a[2] = 5;
//		a[3] = 6;
//		a[4] = 7;
//	for(int i = 0; i < 5; i++)
//		printf("%d  ",a[i]);
//
//	void *dir = opendir("bin/");
//	struct dirent *temp =NULL;
//	do{
//	temp = readdir(dir);
//	//printf("\n printing the directory");
//	if(temp!=NULL)
//		printf("temp:%s\n", temp->d_name);
//	}while(temp!=NULL);
//	for(int64_t i = 0; i<10; i++){
//			buff[i] = 0;
//		}
//	read(0, buff, 8);
//	printf("we read:%s\n", buff);

//	temp = readdir(dir);
//	if(temp !=NULL)
//		printf("temp:%s", temp->d_name);
//	else{
//		printf("\n this is nill");
//	}
//	read(fd, buff, 5);
//	printf("write:::%s", buff);
//	read(fd, buff, 7);
//	printf("write2:::%s", buff);
//	int *a = (int *)malloc(5*sizeof(int));
//	a[0] = 2;
//	a[1] = 4;
//	a[2] = 5;
//		a[3] = 6;
//		a[4] = 7;
//	for(int i = 0; i < 5; i++)
//		printf("%d  ",a[i]);
//
//	void *dir = opendir("bin/");
//	struct dirent *temp =NULL;
//	do{
//	temp = readdir(dir);
//	if(temp!=NULL)
//		printf("temp:%s\n", temp->d_name);
//	}while(temp!=NULL);
////	temp = readdir(dir);
////	if(temp !=NULL)
////		printf("temp:%s", temp->d_name);
////	else{
////		printf("\n this is nill");
////	}
//	//	while(1);
	return 0;
}
