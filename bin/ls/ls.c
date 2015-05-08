#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void noop(char *c) {

}
int main(int argc, char *argv[], char *envp[]) {
	if(argc == 1){
		char *buffer = malloc(100);
		buffer =  getcwd(buffer, 100);
		void *dir = opendir(buffer);
		if(dir == NULL){
			return 0 ;
		}
		struct dirent *temp = NULL;
		printf("\nFILENAME:\n");
		do{
			temp = readdir(dir);
			//printf("\n printing the directory");
			char *name = (char *) malloc(100);
			if (temp != NULL) {
				strcpy(name, temp->d_name);
				int len = strlen(name);
				name[len] = '\0';
				//			name[len] = '\n';
				//			write(1, name, len);
				printf("%s\n", temp->d_name);
			}
		}while(temp!=NULL);
	}
	else{
		void *dir = opendir(argv[1]);
		if(dir == NULL){
			return 0;
		}
		struct dirent *temp = NULL;
		printf("\nFILENAME:\n");
		do {
			temp = readdir(dir);
			//printf("\n printing the directory");
			char *name = (char *) malloc(100);

			if (temp != NULL) {
				strcpy(name, temp->d_name);
				int len = strlen(name);
				name[len] = '\0';
				//			name[len] = '\n';
				//			write(1, name, len);
				printf("%s\n", temp->d_name);
			}
		} while (temp != NULL);
	}
}
