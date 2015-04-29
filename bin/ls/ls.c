#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[], char *envp[]){
	if(argc < 1){
		write(2, "error: ls", 10);
	}
		void *dir = opendir("bin/");
		struct dirent *temp =NULL;
		do{
		temp = readdir(dir);
		//printf("\n printing the directory");
		if(temp!=NULL)
			printf("temp:%s\n", temp->d_name);
		}while(temp!=NULL);
}
