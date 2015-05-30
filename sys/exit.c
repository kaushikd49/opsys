
#include<sys/defs.h>
void exit(int code){
	uint64_t n=60;
	uint64_t result;

	__asm__ __volatile__ (
		"int $0x80"
		:"=a"(result)
		:"0"(n),"D"(code));
	

}
