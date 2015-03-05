#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h>

int printHexIntTime(int n);
int write_char_to_vid_mem(char c, uint64_t pos);
void write_to_video_memory(const char* str, uint64_t position);

//try optimizing this function. see if we need to use a more refined way.
void print_time() {
	// with a frequency of 18.2065 Hz, a interrupt is sent every .0549254 seconds so a second happens every 18.2 calls.
	static int seconds_boot = 0;
	static int ms_boot = 0;
// static int lost_precision = 0; //for the .2 so every 10 increment increment ms_boot once more
	//printf("%x", time);
// lost_precision++;
	ms_boot = ms_boot + 1;
// if(lost_precision == 9) //can optimize
//		ms_boot++;

	if (ms_boot < 18) {
		return;
	} else {
		ms_boot = ms_boot % 2; //can optimize
		seconds_boot = seconds_boot + 1;
		printHexIntTime(seconds_boot);
	}
}
//-------------------------------------------------------------ISR-----------------------------------------------------------------
//exactly like the GDTR. todo: change the intitialization to the way it is done for gdt
struct lidtr_t { //initializing ldtr register
	uint16_t size;
	uint64_t base;
}__attribute__((packed));
struct idtD {
	uint16_t offset_1;
	uint16_t segment_selector;
	char zero;
	char type;
	uint16_t offset2;
	uint32_t offset3;
	uint32_t zero2;
}__attribute__((packed));
//parameters: the base address of the isr table, the interupt number we are mapping, handler function, type of the interrupt(trap, interrupt), segment of the handler)
void add_int_handler(uint64_t isr_base, uint64_t isr_number,
		uint64_t handler_name, char type, uint16_t segment_selector) {
	uint16_t offset1 = (uint16_t)(handler_name & 0x000000000000FFFF);
	uint16_t offset2 = (uint16_t)(handler_name >> 16 & 0x000000000000FFFF);
	uint32_t offset3 = (uint32_t)(handler_name >> 32 & 0x00000000FFFFFFFF);
	struct idtD *isr_base1 = (struct idtD *) isr_base;
	isr_base1[isr_number].offset_1 = offset1;
	isr_base1[isr_number].segment_selector = segment_selector;
	isr_base1[isr_number].zero = 0;
	isr_base1[isr_number].type = type;
	isr_base1[isr_number].offset2 = offset2;
	isr_base1[isr_number].offset3 = offset3;
	isr_base1[isr_number].zero2 = 0;
}

//offset 1 at 32 and offset 2 at 40 because first 31 used by exceptions Hardcoding them for now
void config_PIC() {
	unsigned char PIC1data, PIC2data;

	__asm__ __volatile__(
			"inb $0x21,%0\n\t"
			:"=a"(PIC1data));
	//printf("sdasd: %x",PIC1data);
	__asm__ __volatile__(
			"inb $0xA1,%0\n\t"
			:"=a"(PIC2data));
	//printf("pic2: %x", PIC2data);
	//the steps are done in order.
	__asm__ __volatile__("movb $0x11, %al\n\t"
			"outb %al, $0x20 \n\t"
			"movb $0x11, %al\n\t"
			"outb %al, $0xA0 \n\t"
			"movb $0x20, %al\n\t"
			"outb %al, $0x21 \n\t" //32
			"movb $0x28, %al\n\t"
			"outb %al, $0xA1 \n\t"//32+8
			"movb $4, %al\n\t"
			"outb %al, $0x21 \n\t"//set bit for the cascade
			"movb $2, %al\n\t"
			"outb %al, $0xA1\n\t"//this is just the position where it is cascaded for the pic to know
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
extern void trap_default();
extern void isr_timer();
extern void isr_keyboard();
extern void keyboard_init();
extern void init_keyboard_map();

void init_IDT(struct lidtr_t IDT) {
	uint32_t i = 0;
//	for (i = 0; i < 256; i++) {
//		add_int_handler((uint64_t) IDT.base, i, (uint64_t) isr_default, 0xEF,
//				0x08);  //everything set as trap.
//		//__asm__ __volatile__("INT $0");
//	}
	//setting trap
	for (i = 0; i <32; i++){
		add_int_handler((uint64_t) IDT.base, i, (uint64_t) trap_default, 0xEF, 0x08);//note: we are making the type as P|RING 3|0|TRAP GATE(1111)
	}
	for ( i =32; i < 256; i++){
		add_int_handler((uint64_t) IDT.base, i, (uint64_t) isr_default, 0xEE, 0x08);//note: we are making type as P|RING 3|0|INTERRUPT GATE(1110)
	}
}

void start(uint32_t* modulep, void* physbase, void* physfree) {
	char str[] = "__its_!@#$%^&*()_dangerous__";
//	printf("Welcome to your own OS %d %x %x %d %d %c %x %s %p %p\n", -2147483648, -2147483648, 0, 0x80000000, 0x7fffffff, 'e', 0xa35d,
	for (int i = 0; i < 60; i++)
		printf("%s~~%d\n", str, i);
//	printf("pri\rnting all ascii\n");
//	for (int i = 0; i < 256; i++)
//		printf("%d:%c", i, i);

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
	flags = flags & (1 << 9);
	//printf("hello: %d",flags);

	//while

	//printf("done");
	while (1)
		;
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
//struct lidtr_t IDT;
static struct lidtr_t lidtr = {
	0x1000,
	(uint64_t)(idt_tab),
};
void init_init_IDT() {
	__asm__ __volatile("lidt (%0)"
			:
			:"a"(&lidtr));
	init_IDT(lidtr);
}
void add_custom_interrupt() {
	add_int_handler((uint64_t) lidtr.base, 32, (uint64_t) isr_timer, 0xEE, 0x08); //mode set to present|Ring 3|Interrupt   - Segment usual kernel segments
	add_int_handler((uint64_t) lidtr.base, 33, (uint64_t) isr_keyboard, 0xEE,0x08);
}
void boot(void) {
	// note: function changes rsp, local stack variables can't be practically used
	//register char *s;
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
	init_keyboard_map();
	keyboard_init();
	//printf("%x", 15);
	//print_time();
	__asm__ __volatile("INT $32\n\t"
								   "cli");
	__asm__ __volatile("INT $33\n\t""cli");
	__asm__ __volatile__ ("movb $0xFC, %al\n\t"
			"outb  %al, $0x21\n\t");
	__asm__ ("sti");
	start(
			(uint32_t*) ((char*) (uint64_t) loader_stack[3] + (uint64_t)
					& kernmem - (uint64_t) & physbase), &physbase,
			(void*) (uint64_t) loader_stack[4]);
	//s = "!!!!! start() returned !!!!!";
	//for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	//printf(s);
	while (1)
		;
}
