#include <stdio.h>
#include<stdlib.h>

int arr[100];
int b = 10;

void func(int param) {
	pid_t res;
	res = fork();
	if (res == 0) {
		param = -param;
		printf("%d\n", param);
	} else if (res > 0) {
		func(param + 1);

	} else {
		printf("Error, kill somne\n");
		sleep(100);
	}

}

int main(int argc, char* argv[], char* envp[]) {
	func(5);
	return 0;
}
