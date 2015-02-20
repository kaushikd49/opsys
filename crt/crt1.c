#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]);

void _start(void) {
	uint64_t addr;
	__asm__ __volatile__(
			"mov %%rsp, %%rax\n\t"
			:"=a"(addr)::);
	int step = 8;
	uint64_t argcAddr = addr + step;
	int argc = *((int *) argcAddr);
	uint64_t argvAddr = addr + 2 * step;
	char **argv = ((char **) argvAddr);
	uint64_t envpAddr = argvAddr + step * (1 + argc); //argv is terminated by null
	char **envp = ((char **) envpAddr);
	int res;
	res = main(argc, argv, envp);
	exit(res);
}
