#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
//#include "stdlib.c"
/*bad hack */
int printInteger(int n){
	if(n == 0)
		return 0;
	int temp = n;
	int rem;
	char *apointer, a;
	rem = temp%10;
	a=48+rem;
	apointer=&a;
	int prevcount = printInteger(temp/10);
	write(1,apointer,1);
	return prevcount+1;

}
int printf(const char *format, ...) {
	va_list val;
	int printed = 0;

	va_start(val, format);

	while(*format) {
		if(*format == '%'){
			++format;
			if(*format == '%'){
				write(1, format, 1);
				++printed;
				++format;
			}
			else if(*format == 'd'){
				int tempd = va_arg(val, int);
				int count = printInteger(tempd);
				printed = printed+count;
				++format;
			}
			else if(*format == 's'){
				char *temps = va_arg(val, char *);
				int length = strlen(temps);
				write(1, temps, length);
				printed = printed+length;
				++format;
			}
		}
		else{
			write(1, format, 1);
			++printed;
			++format;
		}
	}

	return printed;
}
