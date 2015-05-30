#include<stdio.h>
#include<stdlib.h>
int main(){
	char *buffer = malloc(0x1000);
	while(1){
		int count = read(0, buffer, 0x1000);
		printf("count: %d", count);
		if(count < 0x999){
			break;
		}
	}
	return 0;
}
