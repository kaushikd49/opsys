#ifndef _STDLIB_C
#define _STDLIB_C
#include<syscall.h>
//#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#define ALIGNMENT 16
#define DEFAULT_MODE 00744
//typedef uint64_t size_t;
//typedef int32_t pid_t;
extern __thread int errno;
typedef char * char_ptr;

struct blockHeader {
	uint64_t size;
	struct blockHeader *next;

};
//does not have an errno to set accouring to gnu standard: http://www.gnu.org/software/libc/manual/html_node/Exit-Status.html
void exit(int status) {
	syscall_1(SYS_exit, status);
}
//check if we have to do PIPE ERRORS: EAGAIN if O_NONBLOCK is set,2)if interupted by a signal. http://linux.die.net/man/3/read
//EBADMSG ?? LOT OF ERRNOS did not check
ssize_t read(int fd, void *buf, size_t count) {
	size_t size = syscall_4_write(SYS_read, fd, buf, count);//we could change this to use the generic syscall_4 but why break.
	if(size == 0xFFFFFFFFFFFFFFF7){
		errno = EBADF;
		return -1;
	}
	else if((signed long)size <0){
		errno = READERR;
		return -1;
	}
	//if(size == 0x)
	return size;
}
//write same issues as read
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
	if(size == 0xFFFFFFFFFFFFFFF7){
		errno = EBADF;
		return -1;
		}
	else if((signed long)size <0){
		errno = WRITEERR;
		return -1;
	}
	return size;
}
//no errors for strlen
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
	int count = 0, i = 10, neg = 0;
	char number[11]="0000000000"; // log(2^31-1) + sign char

	if (n < 0) {
		neg = 1;
		n *= -1;
	} else if (n > 0) {
		while (n != 0) {
			char rem = (n % 10) + '0';
			number[i--] = rem;
			n /= 10;
			count++;
		}
	} else {
		i -= 2;
		count++;
	}

	if (neg) {
		number[i--] = '-';
		count++;
	}
	char *ptr = number + i + 1;
	write(1, ptr, count);
	return count;
}
//do a check for errors after complete function

int printHexInt(int n) {
	char res[] = "00000000", c = '0';
	int base = 0xf, i = 7, new_n = n, j = 0;
	while (new_n != 0) {
		int nibble = (0xf) & (base & new_n) >> (4 * j);
		if (nibble >= 10) {
			c = (nibble + 87);
		} else {
			c = (nibble + 48);
		}
		j++;
		res[i--] = c;
		new_n = new_n & ~base;
		base = base << 4;
	}

	int count = 7 - i;
	if (count > 0) {
		char *ptr = res + i + 1;
		write(1, ptr, count);
	}
	return i;
}

int printf(const char *format, ...) {
	va_list val;
	int printed = 0;

	if (*format != '%') {
		while (*format != '\0') {
			write(1, format, 1);
			printed++;
			format++;
		}
		return printed;
	}
	
	va_start(val, format);

	while(*format && *(format+1)) {
		if(*format == '%') {
			format++;
			char character = *format;

			if (character == 'd') {
				int tempd = va_arg(val, int);
				int count = printInteger(tempd);
				printed = printed + count;
			} else if (character == 'x') {
				int tempd = va_arg(val, int);
				int count = printHexInt(tempd);
				printed = printed + count;
			} else if (character == 's') {
				char *temps = va_arg(val, char *);
				int length = strlen(temps);
				write(1, temps, length);
				printed = printed + length;
			} else if (character == 'c') {
				// char promoted to int in va_arg
				char tempc = va_arg(val, int);
				write(1, &tempc, 1);
				printed++;
			}
		} else {
			write(1, format, 1);
			printed++;
		}
		format++;
	}

	while(*format) {
		write(1, format, 1);
		printed++;
		format++;
	}
	va_end(val);
	return printed;
}

int is_int(char c) {
	int ascii = c - '0';
	return c == '-' || (ascii >= 0 && ascii <= 9);
}

int is_hex(char c) {
	char *hexs = "0xXabcdefgABCDEF-";
	int i = 0;

	if (is_int(c))
		return 1;
	for (i = 0; hexs[i] != '\0'; i++) {
		if (c == hexs[i])
			return 1;
	}
	return 0;
}

int isStrChar(char c) {
	return c != ' ' && c != '\t' && c != '\n';
}

int atoi(char_ptr ptrs[]) {
	char *s = ptrs[0], *e = ptrs[1];
	char *temp = (e - 1);
	int flag = 1;

	if (*s == *e) {
		return 0;
	}
	if (*s == '-') {
		flag = -1;
		s++;
		if (*s == *e) {
			return 0;
		}
	}

	int sum = 0, pow = 1;
	while ((temp + 1) != s) {
		char c = *temp;
		sum += ((c - '0') * pow);
		pow *= 10;
		temp--;
	}
	return sum * flag;
}

int atox(char_ptr ptrs[]) {
	int sum = 0, pow = 1, flag = 1;
	;
	char_ptr first = ptrs[0], second = ptrs[0] + 1, last = ptrs[1];

	if (*first == *last)
		return 0;
	if (*first == '-') {
		flag = -1;
		first++;
		if (*first == *last) {
			return 0;
		}
	}

	if (*first == 0 && (*second == 'x' || *second == 'X')) {
		// 0x2342 or 0X2342
		first += 2;
	} else if (*first == 'x' || *first == 'X') {
		//x123 or X3423
		first++;
	} else {
		char_ptr temp = last - 1;
		while ((temp + 1) != first) {
			int val = 0;
			int ascii = (*temp) - '0';
			if (ascii >= 49 && ascii <= 54)
				val = ascii - 39;
			else if (ascii >= 17 && ascii <= 22)
				val = ascii - 7;
			else if (ascii >= 0 && ascii <= 9)
				val = ascii;
			else
				return 0;

			sum += val * pow;
			pow *= 16;
			temp--;
		}
	}
	return sum * flag;
}

void copy_to_str(char_ptr ptrs[], char *str) {
	char *s = ptrs[0], *e = ptrs[1];
	while (s != e) {
		*str = *s;
		s++;
		str++;
	}
	*str = '\0';
}

void scan_token(int* scanned, char_ptr *buffer_ptr, char_ptr ptrs[],
		int (func)(char), int num_chars) {
	ptrs[0] = NULL;
	ptrs[1] = NULL;
	*scanned = *scanned + 1;
	int i = 0;

	while(**buffer_ptr == ' ')
		*buffer_ptr = *buffer_ptr + 1;

	char* temp = *buffer_ptr;
	while (*buffer_ptr && func(**buffer_ptr)
			&& (num_chars == -1 || ++i <= num_chars)) {
		*buffer_ptr = *buffer_ptr + 1;
	}
	ptrs[0] = temp;
	ptrs[1] = *buffer_ptr;

}
//same as printf
int scanf(const char *format, ...) {
	int scanned = 0;
	int limit = 1000;
// static since scanf unread input across calls
	static char buffer[1000];
	static char *buffer_ptr = buffer;

	if(buffer-buffer_ptr==1000)
		buffer_ptr = buffer;
	read(0, buffer, limit);

	va_list val;
	va_start(val, format);
	char_ptr ptrs[2];

	while (*format) {
		if (*format == '%') {
			format++;
			switch (*format) {
			case 'd':
				format++;
				scan_token(&scanned, &buffer_ptr, ptrs, is_int, -1);
				int *int_ptr = va_arg(val, int *);
				*int_ptr = atoi(ptrs);
				break;
			case 's':
				format++;
				//todo: below working and not giving seg fault
				// even if str was allocated lesser space.
				scan_token(&scanned, &buffer_ptr, ptrs, isStrChar, -1);
				char *str = va_arg(val, char *);
				copy_to_str(ptrs, str);
				break;
			case 'x':
				format++;
				scan_token(&scanned, &buffer_ptr, ptrs, is_hex, -1);
				int *hex_ptr = va_arg(val, int *);
				*hex_ptr = atox(ptrs);
				break;
			case 'c':
				format++;
				scan_token(&scanned, &buffer_ptr, ptrs, isStrChar, 1);
				char *chr_ptr = va_arg(val, char *);
				*chr_ptr = *ptrs[0];
				break;
			default:
				break;
			}
		} else {
			format++;
			//return 0;
		}
	}
	va_end(val);
	return scanned;
}

uint64_t get_brk(uint64_t size) {
	return syscall_1_p(SYS_brk, size);

}
//brk done returns -1 on failure and sets ENOMEM
int brk(void *end_data_segment){
	uint64_t memoryStart = get_brk(0);
	uint64_t returnBrk = get_brk((uint64_t) (end_data_segment));//todo: does get_brk return a value?? adding here: in our shell see if we free all mallocs
		//heap overflow check.
	if(returnBrk == memoryStart){
		errno =ENOMEM;
		return -1;
	}
	return 0;

}
//test function
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
//malloc set errno for EMEM
void *malloc(uint64_t size) {
	static struct blockHeader *head = NULL;
	static struct blockHeader *tail = NULL;
	uint64_t memSize = (size + sizeof(struct blockHeader) + (ALIGNMENT - 1))
			& ~(ALIGNMENT - 1); //Source:cracking the coding interview: page 247
//best fit algorithm to use the empty blocks in the middle of the heap
	void *loc = findBest(head, memSize);
	if (loc != NULL) {
		struct blockHeader *metaData = (struct blockHeader *) loc;
		metaData->size = memSize;
		metaData->size = (metaData->size) | 1;
		void *returnAddress = (void *) ((uint64_t) loc
				+ sizeof(struct blockHeader));
		printAllocmemory(head);
		return returnAddress;
	}
//end of best fit

	uint64_t memoryStart = get_brk(0);
	printf("MEMORY START:%d\n", memoryStart);
	//printf("memsize:%d  %d\n",memSize,memoryStart);
	uint64_t newBrk = (uint64_t) (memoryStart + memSize);//todo: does get_brk return a value?? adding here: in our shell see if we free all mallocs
	//heap overflow check.
	int retBrk = brk((void *)newBrk);
	if(retBrk== -1){
		if(errno == ENOMEM){
			//errno == ENOMEM;
			printf("\nmem. Alloc failed. Out of Memory");
			return NULL;
		}

	}
	//printf("RRRRETURNBRK:%d\n",returnBrk);
	//printf("%d\n",get_brk(0));
	struct blockHeader *metaData = (struct blockHeader*) memoryStart;
	metaData->size = memSize;
	metaData->next = NULL;
	metaData->size = (metaData->size) | 1; //flag for whether it is a valid address.
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
//does not have error value, linux defines illegal pointer access as undefined.
void free(void *ptr) {

	struct blockHeader *current = (struct blockHeader *) ((uint64_t) (ptr)
			- sizeof(struct blockHeader));
	current->size = (current->size) & 0xFFFFFFFFFFFFFFFE;
}
char *strcpy(char *dst, char *src) {
	size_t len = 0;
	while (src[len] != '\0') {
		dst[len] = src[len];
		len++;
	}
	dst[len] = '\0';
	return dst;
}

size_t strncmp(char *string1, char *string2, int n) {
	size_t len = 0;
	while (len < n && string1[len] != '\0' && string2[len] != '\0') {
		if (string1[len] != string2[len])
			break;
		len++;
	}
	if ((len == n) || (string1[len] == string2[len]))
		return 0;
	else if (string1[len] > string2[len])
		return (size_t) string1[len];
	else
		return (size_t) string2[len];
}
//source:mode values taken from linux man pages. http://man7.org/linux/man-pages/man2/open.2.html
enum{
	S_IRWXU = 00700,
	S_IRUSR = 00400,
	S_IWUSR = 00200,
	S_IXUSR = 00100,
	S_IRWXP = 00070,
	S_IRGRP = 00040,
	S_IWGRP = 00020,
	S_IRWXG = 00001,
	S_IRWXO = 00007,
	S_IROTH = 00004,
	S_IWOTH = 00002,
	S_IXOTH = 00001
};
//
size_t open(const char *filename, int permission) {
	unsigned short mode;
	if((permission & 0x40)!=0)
		 mode = DEFAULT_MODE; //todo:verify the mode. default rwx
	uint64_t result =  syscall_3(SYS_open, (uint64_t)filename, (uint64_t)permission,(uint64_t)mode);
	if((signed long)result == -EACCES){//checked
		errno =EACCES;//premission denied to access the file
		return -1;
	}
	else if((signed long)result == -ENOENT){//checked
		errno = ENOENT;//file or directory does not exist
		printf("file or directory not present");
		return -1;
	}
	else if((signed long)result == -EEXIST){//checked
		errno = EEXIST;//file already exists returns when using O_CREATE
		printf("file exists");
		return -1;
	}
	else if((signed long)result == -EDQUOT){
		errno = EDQUOT;//quota of open files exceeded by user.
		return -1;
		}
	else if((signed long)result == -EFAULT){
		errno = EFAULT;//bad address (Accessing illegal memory).
		return -1;
	}
	else if((signed long)result == -EFBIG){
		errno = EFBIG;//file we are reading is too large
		return -1;
	}
	else if((signed long)result == -EINTR){
		errno = EINTR;//if a signal handler interrupts an open
		return -1;
	}
	else if((signed long)result == -EISDIR){
			errno = EISDIR;//happens if the program wanted to read or write a directory.
			return -1;
			}
	else if((signed long)result == -ELOOP){
		errno = ELOOP;//too many sybolic links to follow.
		return -1;
	}
	else if((signed long)result == -EMFILE){//checked
		errno = EMFILE;//too many files open by process
		return -1;
	}
	else if((signed long)result == -ENAMETOOLONG){
			errno = ENAMETOOLONG;//too many files open by process
			return -1;
		}
	else if((signed long)result == -ENFILE){
		errno = ENFILE;//the device address does not exist
		return -1;
	}
	else if((signed long)result == -ENODEV){
			errno = ENODEV;//too many files open by process
			return -1;
		}
	else if((signed long)result == -ENOENT){
			errno = ENOENT;//too many files open by process
			return -1;
		}
	else if((signed long)result == -ENOMEM){
		errno = ENOMEM;//insufficient kernel are available
		return -1;
	}
	else if((signed long)result == -ENOSPC){
		errno = ENOSPC;//no way to create pathname as no space for file
		return -1;
	}
	else if((signed long)result == -ENOTDIR){//checked
		errno = ENOTDIR;//pathname not a dir, but O_DIR specified
		return -1;
	}
	else if((signed long)result == -ENXIO){
		errno = ENXIO;// no file is open for reading
		return -1;
	}
	else if((signed long)result == -EOVERFLOW){
		errno = EOVERFLOW;//file is too large to open
		return -1;
	}
	else if((signed long)result == -EPERM){
		errno = EPERM;
		return -1;
	}
	else if((signed long)result == -EROFS){
		errno = EROFS;//pathname refers to a file read-only, write access requested
		return -1;
	}
	else if((signed long)result == -ETXTBSY){
		errno = ETXTBSY;//an executable is being executed and write access req
		return -1;
	}
	else if((signed long)result == -EWOULDBLOCK){
		errno = EWOULDBLOCK;//too many files open by process
		return -1;
	}
	else if((signed long)result <0 ){
		errno = OPENERROR;
		printf("\nopen error");
		return -1;
	}
	return result;
}

pid_t fork(void) {
	pid_t result;
	result = syscall_0(SYS_fork);
	if((pid_t)result == -EAGAIN){
		errno = EAGAIN;
		return -1;
	}
	else if((pid_t)result == -EAGAIN){
		errno = EAGAIN;
		return -1;
	}
	else if((pid_t)result == -ENOMEM){
		errno = ENOMEM;
		return -1;
	}
	return result;
}
int execve(const char *filename, char * const argv[], char * const envp[]) {
	int64_t result;
	result = syscall_3(SYS_execve, (uint64_t) filename, (uint64_t) argv,
			(uint64_t) envp);
	if(result == -E2BIG){
		errno = E2BIG;
		return -1;
	}
	else if(result == -EACCES){
		errno = EACCES;
		return -1;
	}
	else if(result == -EFAULT){
		errno = EFAULT;
		return -1;
	}
	else if(result == -EIO){
		errno = EIO;
		return -1;
	}
	else if(result == -ELOOP){
		errno = ELOOP;
		return -1;
	}
	else if(result == -EMFILE){
		errno = EMFILE;
		return -1;
	}
	else if(result == -ENAMETOOLONG){
		errno = ENAMETOOLONG;
		return -1;
	}
	else if(result == -ENFILE){
		errno = ENFILE;
		return -1;
	}
	else if(result == -ENOENT){
		errno = ENOENT;
		return -1;
	}
	else if(result == -ENOMEM){
		errno = ENOMEM;
		return -1;
	}
	else if(result == -ENOEXEC){
		errno = ENOEXEC;
		return -1;
	}
	else if(result == -ENOTDIR){
		errno = ENOTDIR;
		return -1;
	}
	else if(result == -EPERM){
		errno = EPERM;
		return -1;
	}
	else if(result == -ETXTBSY){
		errno = ETXTBSY;
		return -1;
	}
	errno =EXECVEERROR;
	return -1;
}

char *strchr(const char *s, int c) {
	char *current = (char *) s;
	while (*current != '\0' && *current != c) {
		current++;
	}
	if (*current == c)
		return current;
	return NULL;
}

char *strcat(char *dest, const char *src) {
	size_t length = strlen(dest);
	size_t len = 0;
	while (src[len] != '\0') {
		dest[length + len] = src[len];
		len++;
	}
	dest[length + len] = '\0';
	return dest;
}

pid_t waitpid(pid_t pid, int *stat_loc, int options) {
//struct ruseage info;
	int result;
	result =  syscall_4_wait(SYS_wait4, pid, stat_loc, options);
	if(result <0){
		if(result == -ECHILD){
			errno = ECHILD;
			return -1;
		}
		if(result == -EINTR){
			errno = EINTR;
			return -1;
		}
		if(result == -EINVAL){
			errno = EINVAL;
			return -1;
		}
	}
	return result;
}

pid_t getppid(void) {
	return syscall_0(SYS_getppid);
}

pid_t getpid(void) {
	return syscall_0(SYS_getpid);
}

int chdir(const char* path) {
	int64_t result;
	result =  syscall_1(SYS_chdir, (uint64_t) path);
	if(result <0){
		if(result == -EACCES){
			errno = EACCES;
			return -1;
		}
		else if(result == -EFAULT){
			errno = EFAULT;
			return -1;
		}
		else if(result == -EIO){
			errno = EIO;
			return -1;
		}
		else if(result == -ELOOP){
			errno = ELOOP;
			return -1;
		}
		else if(result == -ENAMETOOLONG){
			errno = ENAMETOOLONG;
			return -1;
		}
		else if(result == -ENOENT){
			errno = ENOENT;
			return -1;
		}
		else if(result == -ENOMEM){
			errno = ENOMEM;
			return -1;
		}
		else if(result == -ENOTDIR){
			errno = ENOTDIR;
			return -1;
		}
	}
	return 0;
}

int close(int handle) {
	int64_t result;
	result = syscall_1(SYS_close, (uint64_t) handle);
	if(result <0){
		if(result == -EBADF){
			errno = EBADF;
			return -1;
		}
		else if(result == -EINTR){
			errno = EINTR;
			return -1;
		}
		else if(result == -EIO){
			errno = EIO;
			return -1;
		}
	}
	return 0;
}

int dup2(int oldfd, int newfd) {
	return syscall_3_dup2(SYS_dup2, oldfd, newfd);
}

int pipe(int pipefd[2]) {
	return syscall_1_p(SYS_pipe, (uint64_t) pipefd);
}
#define NAME_MAX 255
#define MAX_DIR 200
struct dirent {
	long d_ino;
	uint64_t d_off;
	unsigned short d_reclen;
	char d_name[NAME_MAX + 1];
};
struct dir{
	struct dirent *current;
	int fd;
};
typedef struct dir DIR;
void *opendir(const char *name){

	struct dir *returnVal = (struct dir *)malloc(sizeof(struct dir));
	int fd = open(name,0|0x10000);
	if((signed long)fd < 0){
		returnVal = NULL;//errno has been set by open
		return returnVal;
	}
	struct dirent *direntries = (struct dirent *)malloc(MAX_DIR*sizeof(struct dirent));
	syscall_3((uint64_t)SYS_getdents,(uint64_t)fd,(uint64_t)direntries, MAX_DIR*sizeof(struct dirent));
	//struct dirent *temp = (struct dirent *)(direntries);
	//printf("2STR1:%s \n",temp->d_name);
	//printf("2STR1:%d \n",temp->d_off);
	//printf("2STR1:%d \n",temp->d_reclen);

	//struct dirent *temp1 = (struct dirent *)((uint64_t)temp + (uint64_t)temp->d_reclen); //check if you want to use offset
		//printf("2STR1:%s\n",temp1->d_name);

	returnVal->current = direntries;
	returnVal->fd = fd;
	return returnVal;
}
struct dirent *readdir(void *dir){
	DIR *dentry = (DIR *)dir;
	if(dentry == NULL){
		errno = EBADF;
		return NULL;
	}
	struct dirent *returnDirent = dentry->current;
	if(dentry->current->d_reclen == 0)
		return NULL;
	dentry->current = (struct dirent *)((uint64_t)dentry->current+ (uint64_t)dentry->current->d_reclen);
	return returnDirent;
}
int closedir(void *dir){

	DIR *dentry = (DIR *)dir;
	if(dentry == NULL){
		errno = EBADF;
		return -1;
	}
	free(dentry->current);
	int check =  close(dentry->fd);
	if(check <0){
		errno = EBADF;
		return -1;
	}
	return 0;
}
#endif
