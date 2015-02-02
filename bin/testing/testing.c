#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define NULL ((void *)0)

int main(int argc, char *argv[], char *envp[]){
	char buffer[100];
scanf("%s",buffer);
char a[] = "PS1";
strcat(a,buffer+3);
printf("%s",a);
}
