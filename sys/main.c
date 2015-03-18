#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h>
#include <sys/paging.h>

#define MAX_NUMBER_PAGES (1<<18)
#define PAGE_SIZE (1<<12)
#define PAGE_ALIGN (1<<12)
#define CHAR_SIZE (1<<3)
#define MAX_FREELIST_LENGTH (MAX_NUMBER_PAGES/CHAR_SIZE)
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
	uint16_t offset1 = (uint16_t) (handler_name & 0x000000000000FFFF);
	uint16_t offset2 = (uint16_t) (handler_name >> 16 & 0x000000000000FFFF);
	uint32_t offset3 = (uint32_t) (handler_name >> 32 & 0x00000000FFFFFFFF);
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
	for (i = 0; i < 32; i++) {
		add_int_handler((uint64_t) IDT.base, i, (uint64_t) trap_default, 0xEF,
				0x08);//note: we are making the type as P|RING 3|0|TRAP GATE(1111)
	}
	for (i = 32; i < 256; i++) {
		add_int_handler((uint64_t) IDT.base, i, (uint64_t) isr_default, 0xEE,
				0x08);//note: we are making type as P|RING 3|0|INTERRUPT GATE(1110)
	}
}

struct smap_t {
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed)) *smap;
void create_free_list(uint32_t* modulep, char *free_list){
	uint64_t i;
	uint64_t current_index;
	uint64_t current_bit;
	for(i = 0; i < MAX_FREELIST_LENGTH; i++) //char
		free_list[i] = 0;
	printf("\nfree list size%d", i);
	for (smap = (struct smap_t*) (modulep + 2);
				smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
				++smap){
		if (smap->type == 1 /* memory */&& smap->length != 0) {
			uint64_t start = ((smap->base)+PAGE_ALIGN-1)&~(PAGE_ALIGN-1);
			current_index = (start >>3); //right shift 6 for the index
			current_bit = (start & 0xff);
			while (start <(smap->base+smap->length)){
//				printf("\n%x", start);
				if(start + 0x1000 <(smap->base +smap->length))
					free_list[current_index] |= 1<<current_bit;
				current_bit++;
				if(current_bit == 8){
					current_index += 1;
					current_bit = 0;
				}
				start = start + 0x1000;
			}
		}
	}
	free_list[0] = free_list[0] & 0xFE;
}
uint64_t get_free_page(char *free_list){
	int k, i = 0, check = 0;//todo: could optimize to search from previous page given
	for(;i<MAX_FREELIST_LENGTH;i++){
		if((uint64_t)(free_list[i])>0){
			k = 0;
			while(k<8 && (uint64_t)(((1<<k) & free_list[i]))==0){
				k++;

			}
			if(k <8){
				check = 1;
				break;
			}
		}
	}
	if(check == 1){
		uint64_t return_val = (i*8+k)<<12;
		free_list[i] ^= 1<<k;
		return return_val;
	}
	return 0;
}
void return_page(uint64_t page, char *free_list){
	uint64_t page_frame = page>>12;//remove the offset bits
	uint64_t page_index = page_frame / 8; // get the page index in the free list
	uint64_t page_shift = page_frame % 8; // get the shift in the location in the free list
	if((uint64_t)(free_list[page_index] ^ 1<<page_shift) >0){ //just a check to make sure that the user does not give a non-free page
		free_list[page_index] ^= 1<<page_shift;
	}
}
void start(uint32_t* modulep, void* physbase, void* physfree) {
	printf("physbase:%p, physfree:%p\n", physbase, physfree);
//	char str[] = "__its_!@#$%^&*()_dangerous__";
	printf("Welcome to your own OS %d %x %x %d %d %c %x %s %p %p\n",
			-2147483648, -2147483648, 0, 0x80000000, 0x7fffffff, 'e', 0xa35d);
//	for (int i = 0; i < 500; i++)
//		printf("%s~~%d\t", str, i);
//	printf("pri\rnting all ascii\n");
//	for (int i = 0; i < 256; i++)
//		printf("%d:%c", i, i);

	int before = 0x1;
	int after = set_bit(before, 0, 1);
	printf("res before and after %x:%x\n", before, after);

	uint64_t to = 0xbbbbaaaa8bcd;
	uint64_t from = 0xffffffff2edab01f;
	to = extract_bits(from, 16, 31, to, 32, 47);
	printf("copied from %p to %p\n", from, to);

//	struct smap_t {
//		uint64_t base, length;
//		uint32_t type;
//	}__attribute__((packed)) *smap;
	while (modulep[0] != 0x9001){
		modulep += modulep[1] + 2;
	}
	//int check = 0;
	for (smap = (struct smap_t*) (modulep + 2);
			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
			++smap) {
		if (smap->type == 1 /* memory */&& smap->length != 0) {
			printf("Length of memory:%d\n", smap->length);
			printf("Available Physical Memory [%x-%x]\n", smap->base,
					smap->base + smap->length);
		}
	}
	printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here
	printf("\ncreating free list");
	uint64_t temp = (uint64_t)physfree;
	printf("\nlocation of physfree+temp: %x", temp);
	uint64_t free_list_location = (uint64_t)((((uint64_t)physfree)&(~(PAGE_SIZE - 1))) + (PAGE_SIZE));
	printf("\nlocation of free list: %x", free_list_location);
	char *free_list = (char *)(free_list_location);
	create_free_list(modulep, free_list);
	uint64_t ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	return_page(ret, free_list);
	ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	return_page(0x1000, free_list);
	printf("\ndone");
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;
struct idtD idt_tab[255];
//struct lidtr_t IDT;
static struct lidtr_t lidtr = { 0x1000, (uint64_t) (idt_tab), };
void init_init_IDT() {
	__asm__ __volatile("lidt (%0)"
			:
			:"a"(&lidtr));
	init_IDT(lidtr);
}
void add_custom_interrupt() {
	add_int_handler((uint64_t) lidtr.base, 32, (uint64_t) isr_timer, 0xEE,
			0x08); //mode set to present|Ring 3|Interrupt   - Segment usual kernel segments
	add_int_handler((uint64_t) lidtr.base, 33, (uint64_t) isr_keyboard, 0xEE,
			0x08);

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
	__asm__ __volatile__ ("movb $0xFC, %al\n\t"
			"outb  %al, $0x21\n\t");
	__asm__ ("sti");
	start(
			(uint32_t*) ((char*) (uint64_t) loader_stack[3]
					+ (uint64_t) &kernmem - (uint64_t) &physbase), &physbase,
			(void*) (uint64_t) loader_stack[4]);

	//s = "!!!!! start() returned !!!!!";
	//for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	//printf(s);
	while (1)
		;
}
