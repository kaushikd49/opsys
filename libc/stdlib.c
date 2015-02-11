#ifndef _STDLIB_C
#define _STDLIB_C
#include<syscall.h>
//#include <stdlib.h>
#include <stdarg.h>
#define ALIGNMENT 16
//typedef uint64_t size_t;
//typedef int32_t pid_t;

struct blockHeader {
	uint64_t size;
	struct blockHeader *next;

};
size_t write(int fd, const void *buf, size_t count) {
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
	size_t size = syscall_4_write(SYS_write, fd, buf, count);

	return size;
}
size_t strlen(const char *str) {
	size_t current = 0;
	size_t i = 0;
	while (str[i] != '\0') {
		i++;
		current++;
	}
	return current;
}

int printInteger(int n) {
	if (n == 0)
		return 0;
	int temp = n;
	int rem;
	char *apointer, a;
	rem = temp % 10;
	a = 48 + rem;
	apointer = &a;
	int prevcount = printInteger(temp / 10);
	write(1, apointer, 1);
	return prevcount + 1;

}
int printf(const char *format, ...) {
	va_list val;
	int printed = 0;

	va_start(val, format);

	while (*format) {
		if (*format == '%') {
			++format;
			if (*format == '%') {
				write(1, format, 1);
				++printed;
				++format;
			} else if (*format == 'd') {
				int tempd = va_arg(val, int);
				int count = printInteger(tempd);
				printed = printed + count;
				++format;
			} else if (*format == 's') {
				char *temps = va_arg(val, char *);
				int length = strlen(temps);
				write(1, temps, length);
				printed = printed + length;
				++format;
			}
			/*
			 else if(*format == 'p'){
			 void *tempp = va_arg(val,void *);

			 }*/
		} else {
			write(1, format, 1);
			++printed;
			++format;
		}
	}
	va_end(val);
	return printed;
}

uint64_t get_brk(uint64_t size) {
	return syscall_1_p(SYS_brk, size);

}
void printAllocmemory(struct blockHeader *head) {
	struct blockHeader *current = head;
	while (current != NULL) {
		printf("%d %d %d|||||", current, current->size, current->next);
		current = current->next;
	}
	printf("\n");
}
void *findBest(struct blockHeader *head, uint64_t size) {
	if (head == NULL)
		return NULL;
	struct blockHeader *current = head;
	uint64_t snug = ~0x0;
	void *ptr = NULL;
	while (current != NULL) {
		if ((((current->size) & 0x1) == 0)
				&& ((current->size) & (0xFFFFFFFFFFFFFFFE)) >= size) {
			if (snug > (current->size) - size) {
				snug = (current->size) - size;
				ptr = current;
			}
		}
		current = current->next;
	}
	return ptr;
}
void *malloc(uint64_t size) {
	static struct blockHeader *head = NULL;
	static struct blockHeader *tail = NULL;
	uint64_t memSize = (size + sizeof(struct blockHeader) + (ALIGNMENT - 1))
			& ~(ALIGNMENT - 1); //cracking the coding interview: page 247
	//best fit algorithm to use the empty blocks in the middle of the heap
	void *loc = findBest(head, memSize);
	if (loc != NULL) {
		struct blockHeader *metaData = (struct blockHeader *) loc;
		metaData->size = memSize;
		metaData->size = (metaData->size) | 1;
		void *returnAddress = (void *) (loc + sizeof(struct blockHeader));
		printAllocmemory(head);
		return returnAddress;
	}
	//end of best fit

	uint64_t memoryStart = get_brk(0);
	//printf("memsize:%d  %d\n",memSize,memoryStart);
	get_brk((uint64_t) (memoryStart + memSize));//todo: does get_brk return a value?? adding here: in our shell see if we free all mallocs
	//printf("%d\n",get_brk(0));
	struct blockHeader *metaData = (struct blockHeader*) memoryStart;
	metaData->size = memSize;
	metaData->next = NULL;
	metaData->size = (metaData->size) | 1;//flag for whether it is a valid address.
	if (head == NULL) {
		head = metaData;
		tail = metaData;
	} else {
		tail->next = metaData;
		tail = metaData;
	}
	void *returnAddress = (void *) (memoryStart + sizeof(struct blockHeader));
	printAllocmemory(head);
	return returnAddress;

}
void free(void *ptr) {
	struct blockHeader *current = (struct blockHeader *) ((uint64_t) (ptr)
			- sizeof(struct blockHeader));
	current->size = (current->size) & 0xFFFFFFFFFFFFFFFE;
}
size_t strcpy(char *src, char *dst) {
	return 0;
}

void exit(int status) {
	syscall_1(SYS_exit, status);
}

size_t strncmp(char *string1, char *string2, int n) {
	return 0;
}

//todo: open not working
size_t open(char *filename, int permission) {
	return syscall_2(SYS_open, filename, permission);
}

size_t read(size_t handle, void *buffer, size_t nbyte) {
	return syscall_3(SYS_read, (uint64_t) handle, (uint64_t) buffer,
			(uint64_t) nbyte);
}

pid_t fork(void) {
	pid_t result;
	result = syscall_0(SYS_fork);
	return result;
}
int execve(const char *filename, char * const argv[], char * const envp[]) {
	return syscall_3(SYS_execve, (uint64_t) filename, (uint64_t) argv,
			(uint64_t) envp);
}

char *strchr(const char *s, int c) {
	return 0;
}

char *strcat(char *dest, const char *src) {
	return 0;
}

pid_t waitpid(pid_t pid, int *stat_loc, int options) {
	//struct ruseage info;
	return syscall_4_wait(SYS_wait4, pid, stat_loc, options);
}

pid_t getppid(void) {
	return syscall_0(SYS_getppid);
}

pid_t getpid(void) {
	return syscall_0(SYS_getpid);
}

int chdir(const char* path) {
	return syscall_1(SYS_chdir, (uint64_t) path);
}

int close(int handle) {
	return syscall_1(SYS_close, (uint64_t) handle);
}

int scanf(const char *format, ...) {
	return 0;
}

int dup2(int oldfd, int newfd) {
	return 0;
}

int pipe(int pipefd[2]) {
	return 0;
}
#endif
