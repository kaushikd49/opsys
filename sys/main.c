#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h> // todo: check if importing this here is allowed
//#include "isrhandler_default.c"
uint64_t cursor_pos = 0xb8000;

void write_to_video_memory(const char* str) {
	// todo : register char will end up being used
	// lot of times. Not advisable to make it register.
	register char *s, *v;
	s = (char*) str;
	for (v = (char*) cursor_pos; *s; ++s, v += 2){
		*v = (*s);
		*(v+1) = 0x21;
	}
	cursor_pos = (uint64_t) v;
}
void write_to_video_memory_time(const char* str) {
	register char *s, *v;
		s = (char*) str;
		uint64_t cursor_pos = 0xb8ef0;
		for (v = (char*) cursor_pos; *s; ++s, v += 2){
			*v = (*s);
			*(v+1) = 0x21;
		}
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
//-------------------------TIME TESTING-------------------------------------------------------------------------------------------
//int printIntegerTime(int n) {
//	int count = 0, i = 10, neg = 0;
//	char number[11] = "0000000000"; // log(2^31-1) + sign char
//
//	if (n < 0) {
//		neg = 1;
//		n *= -1;
//	} else if (n == 0) {
//		i -= 2;
//		count++;
//	}
//
//	while (n != 0) {
//		char rem = (n % 10) + '0';
//		number[i--] = rem;
//		n /= 10;
//		count++;
//	}
//
//	if (neg) {
//		number[i--] = '-';
//		count++;
//	}
//	char *ptr = number + i + 1;
//	write_to_video_memory_time(ptr);
//	return count;
//}
void write_hex_time(int count, char* ptr) {
	int newcount = count + 2;
	char newchar[newcount];

	newchar[0] = '0';
	newchar[1] = 'x';
	for (int i = 2; i < newcount; i++, ptr++) {
		newchar[i] = *ptr;
	}
	write_to_video_memory_time(newchar);
}
int printHexaTime(int last_index, int n, char res[]) {
	char c = '0';
	int base = 0xf, i = last_index, new_n = n, j = 0;
	int count = 0;
	if (n == 0) {
		char* zero = "0x0";
		write_hex_time(1, zero);
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
			write_hex_time(count, ptr);
		}
	}
	return count;
}
int printHexIntTime(int n) {
	char res[] = "00000000";
	int last_index = 7;
	int count = printHexaTime(last_index, n, res);
	return count + 2;
}
//try optimizing this function. see if we need to use a more refined way.
void print_time(){
	//with a frequency of 18.2065 Hz, a interrupt is sent every .0549254 seconds so a second happens every 8.2 calls.
	static int seconds_boot=0;
	static int ms_boot=0;
	static int lost_precision = 0; //for the .2 so every 10 increment increment ms_boot once more
	//printf("%x", time);
	lost_precision++;
	ms_boot = ms_boot + 1;
	if(lost_precision == 9) //can optimize
		ms_boot++;

	if(ms_boot < 18){
		return;
	}
	else{
		ms_boot = ms_boot%2; //can optimize
		seconds_boot = seconds_boot+1;
		printHexIntTime(seconds_boot);
	}
}
//void printf(const char *format, ...) {
//	write_to_video_memory(format);
//
//-------------------------------------------------------------ISR-----------------------------------------------------------------
//exactly like the GDTR. todo: change the intitialization to the way it is done for gdt
struct lidtr_t {//initializing ldtr register
	uint16_t size;
	uint64_t base;
}__attribute__((packed));
struct idtD{
	uint16_t offset_1;
	uint16_t segment_selector;
	char zero;
	char type;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t zero2;
}__attribute__((packed));
void add_int_handler(uint64_t isr_base, uint64_t isr_number, uint64_t handler_name, char type, uint16_t segment_selector){
	uint16_t offset1 = (uint16_t)(handler_name & 0x000000000000FFFF);
	uint16_t offset2 = (uint16_t)(handler_name>>16 & 0x000000000000FFFF);
	uint32_t offset3 = (uint32_t)(handler_name>>32 & 0x00000000FFFFFFFF);
	struct idtD *isr_base1 = (struct idtD *)isr_base;
	isr_base1[isr_number].offset_1 = offset1;
	isr_base1[isr_number].segment_selector = segment_selector;
	isr_base1[isr_number].zero = 0;
	isr_base1[isr_number].type = type;
	isr_base1[isr_number].offset2 = offset2;
	isr_base1[isr_number].offset3 = offset3;
	isr_base1[isr_number].zero2 = 0;
}


//offset 1 at 32 and offset 2 at 40 because first 31 used by exceptions Hardcoding them for now
void config_PIC(){
	unsigned char PIC1data, PIC2data;

	__asm__ __volatile__(
					"inb $0x21,%0\n\t"
					:"=a"(PIC1data));
	//printf("sdasd: %x",PIC1data);
	__asm__ __volatile__(
						"inb $0xA1,%0\n\t"
						:"=a"(PIC2data));
	//printf("pic2: %x", PIC2data);
	__asm__ __volatile__("movb $0x11, %al\n\t"
						"outb %al, $0x20 \n\t"
						"movb $0x11, %al\n\t"
						"outb %al, $0xA0 \n\t"
						"movb $0x20, %al\n\t"
						"outb %al, $0x21 \n\t"//32
						"movb $0x28, %al\n\t"
						"outb %al, $0xA1 \n\t"//32+8
						"movb $4, %al\n\t"
						"outb %al, $0x21 \n\t"  //set bit for the cascade
						"movb $2, %al\n\t"
						"outb %al, $0xA1\n\t"  //this is just the the position where it is cascaded for the pic to know
						"movb $0x01, %al\n\t"
						"outb %al, $0x21\n\t"
						"movb $0x01, %al\n\t"
						"outb %al, $0xA1\n\t"
						);
	__asm__ __volatile__("outb %0,$0x21\n\t"
					     :: "a"(PIC1data));
	__asm__ __volatile__("outb %0,$0xA1\n\t"
						 ::"a"(PIC2data));


}
extern void isr_default();
extern void isr_timer();
void init_IDT(struct lidtr_t IDT){
	uint32_t i = 0;
	for( i = 0;i<256;i++){
		add_int_handler((uint64_t)IDT.base,i,(uint64_t)isr_default,0xEF,0x08);//everything set as trap.
		//__asm__ __volatile__("INT $0");
	}
}

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
			//printf("Available Physical Memory [%x-%x]\n", smap->base,
			//		smap->base + smap->length);
		}
	}
	//printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here
	unsigned long flags;
	__asm__ __volatile__ (
							"pushf\n\t"
							"pop %0"
							:"=g"(flags));
	flags = flags & (1<<9);
	//printf("hello: %d",flags);

	//while

	printf("done");
	while(1);
	//__asm__ __volatile__(
		//				"int $55\n\t"
		//				 "cli");
	//int i = 0;
	//int j = 5/i;
	// printf("%d", j);
	//add_int_handler((uint64_t)&_binary_tarfs_end, 31, (uint64_t)isr_default, 0x8E, 0x08);
//	int j = 0;
//	//int i = 2/j;
//	printf("%d", i);
	//__asm__ __volatile__("INT $52");
	//int n = 34;
	/*
	uint64_t result;
	__asm__ __volatile__("xor %%rbx, %%rbx\n\t"
						"syscall"
						:"=a"(result)
						:"0"(n)
						:"rbx");*/
	/*
	*/
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;
struct idtD idt_tab[255];
struct lidtr_t IDT;

void init_init_IDT(){
	IDT.size = 0x1000;//hex(256*16)
	IDT.base = (uint64_t)(idt_tab);
	__asm__ __volatile("lidt (%0)"
						:
						:"a"(&IDT));
	init_IDT(IDT);
}
void add_custom_interrupt(){
	add_int_handler((uint64_t)IDT.base, 32, (uint64_t)isr_timer, 0xEE,0x08); //mode set to present|Ring 3|Interrupt   - Segment usual kernel segments
	add_int_handler((uint64_t)IDT.base, 33, (uint64_t)isr_keyboard, 0xEE, 0x08);
}
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
	init_init_IDT();
	config_PIC();
	add_custom_interrupt();
	//printf("%x", 15);
	//print_time();
	__asm__ __volatile__ ("movb $0xFE, %al\n\t"
							  "outb  %al, $0x21\n\t");
	__asm__ ("sti");
	start(
			(uint32_t*) ((char*) (uint64_t) loader_stack[3] + (uint64_t)
					& kernmem - (uint64_t) & physbase), &physbase,
			(void*) (uint64_t) loader_stack[4]);
	s = "!!!!! start() returned !!!!!";
	//for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	printf(s);
	while (1);
}
