
#include <sys/sbunix.h>
#include <stdarg.h>
#include <stdlib.h>

uint64_t BASE_CURSOR_POS = 0xb8000;
uint64_t cursor_pos = 0xb8000; // todo use above constant
uint64_t PRINT_CONTINIOUS = 0;
int VIDEO_COLS = 80;
int VIDEO_ROWS = 50;
int VID_COLS_WIDTH = 2 * 80;
int MAX_VID_ADDR = 0xb8000 + (80 * 50 * 2);

uint64_t TIMER_LOC = 0xb8ef0;

char* clear_on_overflow(uint64_t position, register char* v) {
	// clearing on overflow
	if (position == PRINT_CONTINIOUS) {
		if (v - ((char*) TIMER_LOC - 1) >= 0) {
			v = (char*) BASE_CURSOR_POS;
			for (int i = 0; i < VIDEO_ROWS * VIDEO_COLS; i++, v += 2) {
				*v = '\0';
			}
			v = (char*) BASE_CURSOR_POS;
		}
	}
	return v;
}

long int curr_line_width(register char* v) {
	return ((v - (char*) BASE_CURSOR_POS) % VID_COLS_WIDTH);
}

void write_to_video_memory(const char* str, uint64_t position) {
	// todo : register char will end up being used
	// lot of times. Not advisable to make it register.
	register char *s, *v;

	if (position == PRINT_CONTINIOUS) {
		v = (char*) cursor_pos;
	} else {
		v = (char*) position;
	}

	s = (char*) str;
	for (; *s; ++s, v += 2) {
		// clearing on overflow
		v = clear_on_overflow(position, v);
		if (*s == '\n' || *s == '\f') {
			// offset to go to next-line
			int offset = (VID_COLS_WIDTH - 2) - curr_line_width(v);
			v = v + offset;
			v = clear_on_overflow(position, v);
		} else if (*s == '\v') {
			// vertical tab - same col on next row
			int offset = VID_COLS_WIDTH - 2;
			v = v + offset;
			v = clear_on_overflow(position, v);
		} else if (*s == '\t') {
			v = v + 8;
			v = clear_on_overflow(position, v);
		} else if (*s == '\r') {
			int offset = curr_line_width(v) + 2;
			if (offset > 0) {
				v = v - offset;
				v = clear_on_overflow(position, v);
			}
		}
		else
		{
			*v = (*s);
			*(v + 1) = 0x02;
		}
	}
	if (position == PRINT_CONTINIOUS) {
		cursor_pos = (uint64_t) v;
	}
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
				uint64_t tempptr = (uint64_t)va_arg(val, void *);
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

