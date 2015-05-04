#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void noop(char *c){

}
int main(int argc, char *argv[], char *envp[]){
	if(argc < 2){
		char str[15] = "error: ls";//todo: handle rodata in kernel
		write(2, str, 15);
		return 0;
	}
		void *dir = opendir(argv[1]);
		struct dirent *temp =NULL;
		do{
		temp = readdir(dir);
		//printf("\n printing the directory");
		char name[100];
		if(temp!=NULL){
			strcpy(name, temp->d_name);
			int len = strlen(name);
			write(1, name, len);
//			printf("temp:%s ", temp->d_name);
		}
		}while(temp!=NULL);

}
