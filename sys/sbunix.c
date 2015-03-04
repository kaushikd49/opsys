#include <sys/sbunix.h>
#include <stdarg.h>
#include <stdlib.h>

#define  VIDEO_COLS 80
#define  VIDEO_ROWS 50
#define  VID_COLS_WIDTH (2 * VIDEO_COLS)
//static const int MAX_VID_ADDR = 0xb8000 + (VIDEO_ROWS * VIDEO_COLS * 2);

uint64_t BASE_CURSOR_POS = 0xb8000;
uint64_t PRINT_CONTINIOUS = 0;
uint64_t TIMER_LOC = 0xb8ef0;

// 3 pages (excluding last line for timer)
// worth of characters can be buffered.
#define  NUM_SCREENS 3
#define  VID_MEM_WRITABLE (VIDEO_COLS * (VIDEO_ROWS - 1)) // reserving last row for timer
#define  VID_BUFFER_SIZE (VID_MEM_WRITABLE * NUM_SCREENS)
#define  MAX_CHARS_IN_VID_BUFFER (VID_BUFFER_SIZE / 2) // 1 byte char and other for color
char video_buffer[VID_BUFFER_SIZE];

//uint64_t cursor_pos = BASE_CURSOR_POS;
// tracks last data in the buffer
char * vid_buffer_tail_ptr = video_buffer;
// tracks the window that is written into video mem
char * vid_buffer_view_ptr = video_buffer;

void write_char_color(char * v, char s) {
	*v = s;
	*(v + 1) = 0x02;
}

//void clear_screen() {
//	register char* v = (char*) BASE_CURSOR_POS;
//	for (int i = 0; i < VIDEO_ROWS * VIDEO_COLS; i++, v += 2) {
//		*v = '\0';
//	}
//}

//char* clear_on_overflow(uint64_t position, register char* v) {
//	// clearing on overflow
//	if (position == PRINT_CONTINIOUS) {
//		if (v - ((char*) TIMER_LOC - 1) >= 0) {
//			clear_screen();
//			v = (char*) BASE_CURSOR_POS;
//		}
//	}
//	return v;
//}

long int curr_line_width(register char* v) {
	return ((v - (char*) BASE_CURSOR_POS) % VID_COLS_WIDTH);
}

long int curr_line_char_width(char* v) {
	return ((v - (char*) BASE_CURSOR_POS) % VIDEO_COLS);
}

void update_buffer_ptrs(char* q) {
	vid_buffer_tail_ptr = q;
	vid_buffer_view_ptr = q;
}

void shift_buffer_on_overflow(int len) {
	int str_bytes = 2 * len;
	// buffer overflow check
	if (vid_buffer_tail_ptr + str_bytes - video_buffer >= VID_BUFFER_SIZE) {
		int offset = str_bytes + 1;
		// shift buffer contents to top
		char *p = video_buffer + offset;
		for (; p <= vid_buffer_tail_ptr; p++) {
			*(p - offset) = *p;
		}
		// reset the void created by shift
		char *q = vid_buffer_tail_ptr;
		for (int k = 0; k < offset; k++, q--) {
			*q = '\0';
		}
		update_buffer_ptrs(q);
	}
}

void repeat_chars_into_buffer(char c, int num) {
	shift_buffer_on_overflow(num);
	char *r = vid_buffer_tail_ptr;
	if (num > 0) {
		for (int i = 0; i < num; i++, r += 2) {
			write_char_color(r, c);
		}
	} else if (num < 0) {
		num *= -1;
		for (int i = 0; i < num; i++, r -= 2) {
			write_char_color(r, c);
		}
	}
	update_buffer_ptrs(r);
}

int strlen(const char *str) {
	int i = 0;
	for (; *str; str++, i++)
		;
	return i;
}

char* handle_oversized_str(const char* s) {
	char *str = (char *) s;
	int len = strlen(str);
	if (len > MAX_CHARS_IN_VID_BUFFER) {
		// truncate str to accommodate it to the vid buffer
		str += len - MAX_CHARS_IN_VID_BUFFER;
	}
	shift_buffer_on_overflow(len);
	return str;
}

void write_string_into_buffer(const char* str) {
	str = handle_oversized_str(str);
	int offset = 0;
	for (char *s = (char *) str; *s; ++s) {
		if (*s == '\n' || *s == '\f') {
			// offset to go to next-line
			offset = (VIDEO_COLS - 1)
					- curr_line_char_width(vid_buffer_tail_ptr);
		} else if (*s == '\v') {
			// vertical tab - same col on next row
			offset = VIDEO_COLS - 1;
		} else if (*s == '\t') {
			offset = 4;
		} else if (*s == '\r') {
			offset = curr_line_char_width(vid_buffer_tail_ptr) + 1;
		} else if (*s == '\b') {
			repeat_chars_into_buffer('\0', -1);
			vid_buffer_tail_ptr -= 2;
		} else {
			repeat_chars_into_buffer(*s, 1);
		}

		if (offset > 0) {
			// *s is a whitespace apart from
			repeat_chars_into_buffer('\0', offset);
			offset = 0;
		}
	}
}

void write_buffer_view_into_vid_mem() {
	// copy a window of size vid_mem from vid_mem_buffer to vid_mem
	char *from = vid_buffer_view_ptr, *to = vid_buffer_view_ptr; //todo: void handle
	char *temp = vid_buffer_view_ptr - VID_MEM_WRITABLE;
	if (temp >= video_buffer) {
		// enough stuff inside buffer to cover video memory
		from = temp;
	} else {
		from = video_buffer;
	}
	char *vid_ptr = (char *) BASE_CURSOR_POS;
	for (char*p = from; p <= to; p++) {
		*vid_ptr = *p;
	}
}

void write_to_video_memory(const char* str, uint64_t position) {
// todo : register char will end up being used
// lot of times. Not advisable to make it register.
	if (position != PRINT_CONTINIOUS) {
		char *v = (char*) position;
		// direct video mem writing, no checks done
		for (; *str; str++, v += 2) {
			write_char_color(v, *str);
		}
		return;
	}
	// first of all, buffer it
	write_string_into_buffer(str);

	write_buffer_view_into_vid_mem(vid_buffer_tail_ptr);
}

int printInteger(int n, uint64_t pos) {
	int count = 0, i = 10, neg = 0;
	char number[] = "00000000000";

	if (n < 0) {
		neg = 1;
	} else if (n == 0) {
		i -= 1;
		count++;
	}

	while (n != 0) {
		int digit = (n % 10);
		if (digit < 0)
			digit *= -1;
		char rem = digit + '0';
		number[i--] = rem;
		n /= 10;
		count++;
	}

	if (neg) {
		number[i--] = '-';
		count++;
	}
	char *ptr = number + i + 1;
	write_to_video_memory(ptr, pos);
	return count;
}

int write_and_get_count(char* ptr, uint64_t pos) {
	write_to_video_memory(ptr, pos);
	int i = 0;
	for (i = 0; ptr[i]; i++)
		;
	return i;
}

char* getHexa(int last_index, int64_t n, char res[], int num_nibbles) {
	char c = '0';
	int i = last_index, j = 0;
	int64_t new_n = n;
	int64_t base = 0xf;
	if (n == 0) {
		res[last_index] = 0;
		return res + last_index - 1;
	} else {
		while (new_n != 0 && j < num_nibbles) {
			int nibble = (0xf) & (base & new_n) >> (4 * j);
			if (nibble >= 10) {
				c = (nibble + 87);
			} else {
				c = (nibble + 48);
			}
			j++;
			res[i--] = c;
			new_n = new_n & ~base;
			base = base << 4;
		}
		return res + i + 1;
	}
}

//do a check for errors after complete function

int printHexInt(int n, uint64_t pos) {
	char res[] = "00000000";
	int last_index = 7;
	char* ptr = getHexa(last_index, n, res, 8);
	return write_and_get_count(ptr, pos);
}

// todo: prints junk. recheck
int printHexUnsignedLong(uint64_t n, uint64_t pos) {
	char res[] = "000000000000000000"; // 16 for ptr val, 2 for 0x
	int last_index = 17;
	char* ptr = getHexa(last_index, n, res, 16);
	*(--ptr) = 'x';
	*(--ptr) = '0';
	return write_and_get_count(ptr, pos);
}

int write_char_to_vid_mem(char c, uint64_t pos) {
	char tempcarray[] = { c, '\0' };
	return write_and_get_count(tempcarray, pos);
}

void printf(const char *format, ...) {
	va_list val;
	va_start(val, format);
	while (*format && *(format + 1)) {
		if (*format == '%') {
			format++;
			char character = *format;

			if (character == 'd') {
				int tempd = va_arg(val, int);
				printInteger(tempd, PRINT_CONTINIOUS);
			} else if (character == 'x') {
				int tempd = va_arg(val, int);
				printHexInt(tempd, PRINT_CONTINIOUS);
			} else if (character == 'p') {
				uint64_t tempptr = (uint64_t) va_arg(val, void *);
				printHexUnsignedLong(tempptr, PRINT_CONTINIOUS);
			} else if (character == 's') {
				char *temps = va_arg(val, char *);
				write_and_get_count(temps, PRINT_CONTINIOUS);
			} else if (character == 'c') {
				// char promoted to int in va_arg
				char tempc = va_arg(val, int);
				write_char_to_vid_mem(tempc, PRINT_CONTINIOUS);
			}
		} else {
			write_char_to_vid_mem(*format, PRINT_CONTINIOUS);
		}
		format++;
	}

	while (*format) {
		write_char_to_vid_mem(*format, PRINT_CONTINIOUS);
		format++;
	}
	va_end(val);
}

int printHexIntTime(int n) {
	return printInteger(n, TIMER_LOC);
}

