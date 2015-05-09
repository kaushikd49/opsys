#include<stdio.h>
#include<stdlib.h>

int myatoi(char s[]) {
	int res = 0;
	int pow = 1;

	for (int i = strlen(s)-1; i >= 0; i--) {
		int c = s[i] - '0';
		if (c < 0 || c > 9) {
			return -1;
		}
		res += pow * c;
		pow *= 10;
	}
	return res;
}

int main(int argc, char *argv[], char *envp[]) {
	printf("\n");
	if (argc != 2 || myatoi(argv[1]) == -1) {
		write(2, "\n[USAGE]: sleep <number>", 25);
		return 1;
	}

	sleep(myatoi(argv[1]));
	//printf("\n woke up after %ds", myatoi(argv[1]));
	return 0;
}
