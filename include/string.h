#ifndef _STRING_H
#define _STRING_H
char *strcpy(char *d,const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(char *string1, char *string2, int n); 
size_t strlen(char *str);
char *strcat(char *dest, const char *src);
#endif
