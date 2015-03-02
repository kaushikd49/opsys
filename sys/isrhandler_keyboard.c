#ifndef ISRHANDLER_KEYBOARD_C
#define ISRHANDLER_KEYBOARD_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>

#define RELEASE_CODE (~15)
#define A_PRESSED 0x1C
#define SHIFT_PRESSED 0x12
extern char keyboard_map[256][3];
void isrhandler_keyboard(){
	char scancode;
	static int released = 0;
	static int caps = 0;
	char printchar;

	__asm__ __volatile__("inb $0x60, %%al\n\t"
						:"=a"(scancode));
	if(scancode == RELEASE_CODE){
			//printf("releassed");
			//printf("release now\n");
			released = 1;
			return;

	}

	//printf("%x\n", scancode);
	//printf("%x\n",RELEASE_CODE);
	if(released == 1){
		released = 0;
		//printf("releasing");
		if(scancode == SHIFT_PRESSED){
			caps = 0;
			//printf("caps released");
		}

	}
	else{
		if(scancode ==SHIFT_PRESSED)
			caps = 0x20;
//		if(scancode == A_PRESSED){
//			printchar = 'a' ^ caps;
//			printf("printing:%c\n", printchar);
//		}
		if(keyboard_map[(int)(scancode)][1] == 1){
			printchar = keyboard_map[(int)(scancode)][0];
			printchar = printchar ^caps;
			printf("printing:%c\n", printchar);
		}
		else if(caps == 0 && keyboard_map[(int)(scancode)][1] == 2){
			printchar = keyboard_map[(int)(scancode)][0];
			printchar = printchar ^caps;
			printf("printing:%c\n", printchar);
		}
		else if(caps !=0 && keyboard_map[(int)(scancode)][1] == 2){
			printchar = keyboard_map[(int)(scancode)][2];
			printf("printing:%c\n", printchar);
		}
	}


}

#endif
