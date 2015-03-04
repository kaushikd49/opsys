#ifndef ISRHANDLER_KEYBOARD_C
#define ISRHANDLER_KEYBOARD_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>

int write_char_to_vid_mem(char c, uint64_t pos) ;
void write_to_video_memory(const char* str, uint64_t position);

#define TAB 0xd
#define ENTER 0x5a
#define SHIFT 0x12
#define BACKSPACE 0x66
#define KEY_RELEASE (~15)


#define A_PRESSED 0x1C

extern char keyboard_map[256][3];
uint64_t glyph_pos = 0xb8f60;

void clear_and_print_str_at(char *str) {
	write_to_video_memory("             ", glyph_pos); // clear space for the glyph tp be writen
	write_to_video_memory(str, glyph_pos);
}

void clear_and_print_char_at(char c) {
	write_to_video_memory("             ", glyph_pos); // clear space for the glyph tp be writen
	write_char_to_vid_mem(c, glyph_pos);
}

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
	} 
	else if (scancode == ENTER) {
		clear_and_print_str_at("<ENT>");
	} 
	else if (scancode == TAB) {
		clear_and_print_str_at("<TAB>");
	} 
	else if (scancode == BACKSPACE) {
		clear_and_print_str_at("<BACKSPACE>");
	} 
	else {
		if (scancode == SHIFT)
			caps = 0x20;

		int int_scancode = (int) (scancode);
		if (keyboard_map[int_scancode][1] == 1) {
			// Print characters
			printchar = keyboard_map[int_scancode][0];
			printchar = printchar ^ caps;
			clear_and_print_char_at(printchar);
		} else if (caps == 0 && keyboard_map[int_scancode][1] == 2) {
			// Print digits
			printchar = keyboard_map[int_scancode][0];
			printchar = printchar ^ caps;
			clear_and_print_char_at(printchar);
		} else if (caps != 0 && keyboard_map[int_scancode][1] == 2) {
			// Print symbols
			printchar = keyboard_map[int_scancode][2];
			clear_and_print_char_at(printchar);
		}
	}
}

#endif
