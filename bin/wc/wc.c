#include<stdlib.h>
#include<stdio.h>

void noop(int count){

}
int main(int argc, char *argv[], char *envp[]){
//	printf("inside wcc");
	char *buffer = malloc(1000000);
//	scanf("%s", buffer);
	read(0, buffer, 1000000);
	printf("back");
	char *start = buffer;
	int count = 0;
	while(*start !='\0'){
		count++;
		start++;
	}
//	char *output = malloc(100);
	//	char *str = "count  = ";
//	char *str = "dasdsdasdasffasggagagagg";
//	write(1, str, 100);
	//	strcpy(output, str);
//	strcat(output, )
	printf("\ncount s:%d",count);
//	noop(count);
}
