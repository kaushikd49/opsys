#include<syscall.h>
//#include <stdlib.h>
#include <stdarg.h>
//typedef uint64_t size_t;
//typedef int32_t pid_t;
size_t strcpy(char *src, char *dst){
	return 0;
}

void exit (int status){
	syscall_1(SYS_exit,status);
}

size_t strlen(const char *str){
	size_t current=0;
	size_t i= 0;
	while(str[i]!='\0'){
		i++;
		current++;
	}
	return current;
}

size_t  strncmp(char *string1, char *string2, int n){
	return 0;
}

size_t open(char *filename, int access, int permission){
	return 0;
}

size_t read(size_t handle, void *buffer, size_t nbyte){
	return 0;
}
size_t write(int fd, const void *buf, size_t count){
	/*
	__asm__ __volatile__ (
													"movq $1, %%rax\n\t"
													"movq $1, %%rdi\n\t"
													"movq %0, %%rsi\n\t"
													"movq $1, %%rdx\n\t"
													"syscall"
													:
													:"r"(hello)
													:"rax", "rdi", "rsi", "rdx", "memory"
													);*/
	size_t size = syscall_4_write(SYS_write, fd,buf,count);

	return size;
}

pid_t fork(void){
	pid_t result;
	result =syscall_0(SYS_fork);
	return  result;
}
int execve(const char *filename, char *const argv[], char *const envp[]){
	return 0;
}

char *strchr(const char *s, int c){
	return 0;
}

char *strcat(char *dest, const char *src){
	return 0;
}

pid_t waitpid(pid_t pid, int *stat_loc, int options){
	//struct ruseage info;
	return syscall_4_wait(SYS_wait4, pid, stat_loc, options);
}

int chdir(const char* path){
	return 0;
}

int close(int handle){
	return 0;
}

int scanf(const char *format, ...){
	return 0;
}

void free(void *ptr){
}

int dup2(int oldfd, int newfd){
	return 0;
}

int pipe(int pipefd[2]){
	return 0;
}

