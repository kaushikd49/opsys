#include <stdio.h>
#include <stdlib.h>
int arr[100];
int b = 10;

void foo(char *s) {

}
int main(int argc, char* argv[], char* envp[]) {
	printf("Hello World! Lets read some brother\n");
	//	char buf[100];
	char * buf = (char *) malloc(10000);
	printf("addr of var is %x, val is %c ", buf + 0x2000);
	foo(buf);
	read(0, buf + 0x2000, 10);
	printf(" read below ");
	write(1, buf + 0x2000, 10);
//	while (1)
//		;
	return 0;
}
