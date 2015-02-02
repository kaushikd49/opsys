#ifndef _STRING_H
#define _STRING_H
char *strcpy(char *d,const char *s);
int strncmp(char *string1, char *string2, int n); 
size_t strlen(char *str);
char *strcat(char *dest, const char *src);
char * strchr (char * str, int character );

void strcpybtwptrs (char *begin, char *end, char *str){
	char *current = begin;
	int i=0;
	while(current+i!=end){
		str[i]=current[i];
		i++;
	}
	str[i]='\0';
}

#endif
