#include<stdio.h>
#include<syscall.h>
#include<../libc/stdlib.c>

void child_sample_exec(char* argv[], char* envp[]) {
	printf("**child now executing ls with chdir**\n");
	chdir("/");
	execve("/bin/ls", argv, envp);
}

void parent_sample_exec(char* argv[], char* envp[]) {
	printf("**parent now executing date**\n");
	execve("/bin/date", argv, envp);
}

void sample_file_readwrite() {
	char *file = "/tmp/temp.txt";
	char *buffer = "we the people of the world";
	printf("attempting to write into %s\n", file);
	// todo: find a way to remove hard-coding of permission below
	int write_fd = open(file, 2);
	write(write_fd, buffer, 10);
	close(write_fd);
}

int main(int argc, char* argv[], char* envp[]) {
//	int a = 12;
//	scanf("%d", &a);
//	printf("%d", a);

//	char s[22];
//	scanf("%s", s);
//	printf("%s", s);

//	int a;
//	scanf("%x", &a);
//	printf("%d", a);

//	char c ;
//	scanf("%c", &c);
//	printf("%c", c);

	int a = 22;
	scanf("%d", &a);
	printf("%x", a);

//	int a = 22;
//	scanf("%x", &a);
//	printf("%x", a);

	exit(0);
	pid_t pid = fork();
	if (pid >= 0) {
		if (pid == 0) {
			char *msg = "inside child bro.\n";
			size_t length = strlen(msg);
			write(1, msg, length);
//			child_sample_exec(argv, envp);
			printf("parent must be %d\n", getppid());

			exit(0);
		} else {
			int status;
			//uint64_t hello = 12;
			waitpid(-1, &status, 0);
			char *str = "hello child";
			printf("\ninside parent process %d. %s\n", getpid(), str);
			//void *ptr = NULL;
			int *ptr = (int *) malloc(sizeof(int) * 10);
			char *ptr1 = malloc(sizeof(char) * 4);
			//int i = 1024244;
			//for (i = 0; i < 10; i++)
			//	ptr[i] = i;
			*ptr1 = 'a';
			*(ptr1 + 1) = 'b';
			*(ptr1 + 2) = 'c';
			*(ptr1 + 3) = '\0';
			/*
			 for(i=0;i<4;i++)
			 printf("%d\t",ptr[i]);
			 printf("\n%s", ptr1);
			 */
			free(ptr);
			int *ptr3 = (int *) malloc(sizeof(int) * 5);
			ptr3[0] = 33;
			/*
			 printf("\nptr3:%d\n",ptr3[0]);
			 */
			sample_file_readwrite();

//			parent_sample_exec(argv, envp);
			//printf("\n");
			//testing strcpy
			//char *ptr4 = (char *)malloc(sizeof(char) * 10);
			//char str4[10] = "S";
			//strcpy(ptr4,str4);
			//printf("\n%s\n", ptr4);
			//testing strncmp(done all possible tests)
			//char *ptr5 = "";
			//char *ptr6 = "ABC";
			//printf("%s",ptr5);
			//printf("ANSWER:%d", strncmp(ptr5,ptr6,3));
			//testing strchr
			//printf("\nSTRCHR:%s",strchr(ptr6,'A'));//dont try a character not there seg fault but thats supposed to happen
			//testing strcat
			//printf("\nSTRCAT:%s", strcat(ptr4,ptr5));
			exit(0);
		}
	} else {
		char msg2[] = "unsuccessful fork bro\n";
		size_t length = strlen(msg2);
		write(1, msg2, length);
	}
	exit(0);

}
