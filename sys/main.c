#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h> // todo: check if importing this here is allowed
uint64_t cursor_pos = 0xb8000;

void write_to_video_memory(const char* str) {
	// todo : register char will end up being used
	// lot of times. Not advisable to make it register.
	register char *s, *v;
	s = (char*) str;
	for (v = (char*) cursor_pos; *s; ++s, v += 2)
		*v = *s;
	cursor_pos = (uint64_t) v;
}

int printInteger(int n) {
	int count = 0, i = 10, neg = 0;
	char number[11] = "0000000000"; // log(2^31-1) + sign char

	if (n < 0) {
		neg = 1;
		n *= -1;
	} else if (n == 0) {
		i -= 2;
		count++;
	}

	while (n != 0) {
		char rem = (n % 10) + '0';
		number[i--] = rem;
		n /= 10;
		count++;
	}

	if (neg) {
		number[i--] = '-';
		count++;
	}
	char *ptr = number + i + 1;
	write_to_video_memory(ptr);
	return count;
}

void write_hex(int count, char* ptr) {
	int newcount = count + 2;
	char newchar[newcount];

	newchar[0] = '0';
	newchar[1] = 'x';
	for (int i = 2; i < newcount; i++, ptr++) {
		newchar[i] = *ptr;
	}
	write_to_video_memory(newchar);
}

int printHexa(int last_index, int n, char res[]) {
	char c = '0';
	int base = 0xf, i = last_index, new_n = n, j = 0;
	int count = 0;
	if (n == 0) {
		char* zero = "0x0";
		write_hex(1, zero);
		count = 1;
	} else {
		while (new_n != 0) {
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
		count = last_index - i;
		if (count > 0) {
			char* ptr = res + i + 1;
			write_hex(count, ptr);
		}
	}
	return count;
}

//do a check for errors after complete function

int printHexInt(int n) {
	char res[] = "00000000";
	int last_index = 7;
	int count = printHexa(last_index, n, res);
	return count + 2;
}

// todo: prints junk. recheck
int printHexUnsignedLong(uint64_t n) {
	char res[] = "0000000000000000";
	int last_index = 15;
	int count = printHexa(last_index, n, res);
	return count + 2;
}

void write_char_to_vid_mem(char c) {
	char tempcarray[] = { c, '\0' };
	write_to_video_memory(tempcarray);
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
				printInteger(tempd);

			} else if (character == 'x') {
				int tempd = va_arg(val, int);
				printHexInt(tempd);
			} else if (character == 'p') {
				uint64_t tempd = va_arg(val, uint64_t);
				printHexInt(tempd);
//				printHexUnsignedLong(tempd);
			} else if (character == 's') {
				char *temps = va_arg(val, char *);
				write_to_video_memory(temps);

			} else if (character == 'c') {
				// char promoted to int in va_arg
				char tempc = va_arg(val, int);
				write_char_to_vid_mem(tempc);
			}
		} else {
			write_char_to_vid_mem(*format);
		}
		format++;
	}

	while (*format) {
		write_char_to_vid_mem(*format);
		format++;
	}
	va_end(val);
}

//void printf(const char *format, ...) {
//	write_to_video_memory(format);
//}

void start(uint32_t* modulep, void* physbase, void* physfree) {
//	printf("Welcome to your own OS \n");
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while (modulep[0] != 0x9001)
		modulep += modulep[1] + 2;
	for (smap = (struct smap_t*) (modulep + 2);
			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
			++smap) {
		if (smap->type == 1 /* memory */&& smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n", smap->base,
					smap->base + smap->length);
		}
	}
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void boot(void) {
	// note: function changes rsp, local stack variables can't be practically used
	register char *s;
	__asm__(
			"movq %%rsp, %0;"
			"movq %1, %%rsp;"
			:"=g"(loader_stack)
			:"r"(&stack[INITIAL_STACK_SIZE])
	);
	reload_gdt();
	setup_tss();
	start(
			(uint32_t*) ((char*) (uint64_t) loader_stack[3] + (uint64_t)
					& kernmem - (uint64_t) & physbase), &physbase,
			(void*) (uint64_t) loader_stack[4]);
	s = "!!!!! start() returned !!!!!";
//	for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	printf(s);
	while (1)
		;
}
