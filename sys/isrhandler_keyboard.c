#ifndef ISRHANDLER_KEYBOARD_C
#define ISRHANDLER_KEYBOARD_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>

#define KEY_RELEASE (~15)
#define SPACE 0x29
#define SHIFT 0x12
#define ENTER 0x5a
#define A_PRESSED 0x1C

extern char keyboard_map[256][3];

void isrhandler_keyboard() {
	char scancode;
	static int key_released = 0;
	static int caps = 0;
	char printchar;

	__asm__ __volatile__("inb $0x60, %%al\n\t"
			:"=a"(scancode));
	if (scancode == KEY_RELEASE) {
		key_released = 1;
		return;
	}

	if (key_released == 1) {
		key_released = 0;
		if (scancode == SHIFT) {
			caps = 0;
		}
	} else if (scancode == ENTER) {
		printf("%c", '\n');
	} else {
		if (scancode == SHIFT)
			caps = 0x20;

		int int_scancode = (int) (scancode);
		if (keyboard_map[int_scancode][1] == 1) {
			// Print characters
			printchar = keyboard_map[int_scancode][0];
			printchar = printchar ^ caps;
			printf("%c", printchar);
		} else if (caps == 0 && keyboard_map[int_scancode][1] == 2) {
			// Print digits
			printchar = keyboard_map[int_scancode][0];
			printchar = printchar ^ caps;
			printf("%c", printchar);
		} else if (caps != 0 && keyboard_map[int_scancode][1] == 2) {
			// Print symbols
			printchar = keyboard_map[int_scancode][2];
			printf("%c", printchar);
		}
	}
}

#endif
