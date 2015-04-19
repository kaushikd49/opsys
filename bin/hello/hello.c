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
int arr[100];
int b =10;

int main(int argc, char* argv[], char* envp[]) {
//	fork();
	printf("Hello World!---%d\n");
//	while(1);
	return 0;
}
