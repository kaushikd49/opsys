#ifndef ISRHANDLER_KEYBOARD_C
#define ISRHANDLER_KEYBOARD_C
#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>

extern char * vid_buffer_view_ptr;
extern char * vid_buffer_tail_ptr;
extern char * video_buffer;
int write_char_to_vid_mem(char c, uint64_t pos);
void write_to_video_memory(const char* str, uint64_t position);
void write_buffer_view_into_vid_mem();

#define TAB 0xd
#define ENTER 0x5a
#define SHIFT 0x12
#define BACKSPACE 0x66
#define KEY_RELEASE (~15)
#define ARROW_KEY_BEGIN -32

#define DOWN 0x72
#define UP 0x75

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
	static int arrow_key = 0;
	char printchar;

	__asm__ __volatile__("inb $0x60, %0\n\t"
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
		clear_and_print_str_at("<ENT>");
	} else if (scancode == TAB) {
		clear_and_print_str_at("<TAB>");
	} else if (scancode == BACKSPACE) {
		clear_and_print_str_at("<BACKSPACE>");
	} else {
		if (scancode == SHIFT) {
			caps = 0x20;
		} else if (scancode == ARROW_KEY_BEGIN) {
			arrow_key = 1;
		} else if (arrow_key && !key_released) {
			//todo reuse constants from other file
			int VID_COLS_WIDTH = 160;
//			int VID_BUFFER_SIZE = 80 * 49 * 3;
			if (scancode == UP) {
				if (vid_buffer_view_ptr - VID_COLS_WIDTH < video_buffer) {
					vid_buffer_view_ptr = video_buffer;
				} else {
					vid_buffer_view_ptr -= VID_COLS_WIDTH;
				}
			} else if (scancode == DOWN) {
				if (vid_buffer_view_ptr + VID_COLS_WIDTH <= vid_buffer_tail_ptr) {
					vid_buffer_view_ptr += VID_COLS_WIDTH;
				}
			}
			write_buffer_view_into_vid_mem();
		} else if (arrow_key && key_released
				&& (scancode == UP || scancode == DOWN)) {
			// arrow key released
			arrow_key = 0;
		}

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
