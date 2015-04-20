#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h>
#include <sys/paging.h>
#include <sys/main.h>
#include <sys/isr_stuff.h>
#include <sys/initkeyboard.h>
#include <sys/printtime.h>
#include <sys/kmalloc.h>
#include <sys/process.h>
#include <sys/system_calls.h>
#include <sys/freelist.h>
#include<sys/pagingglobals.h>
#include<sys/tarfs_FS.h>
static uint64_t current_syscall =0;
static uint64_t rsp_user_syscall;
static uint64_t rip_syscall;
uint64_t * get_physical_pml4_base_for_process();
uint64_t * get_free_frame();
#define INITIAL_STACK_SIZE 4096
extern void _jump_usermode();
extern char video_buffer[4096];
//struct smap_t {
//	uint64_t base, length;
//	uint32_t type;
//}__attribute__((packed)) *smap;
struct smap_t *smap;
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;
struct node {
	int data;
	struct node *next;
};
struct bignode {
	long data1;
	long data2;
	long data3;
	long data4;
	long data5;
	struct bignode *next;
};
struct node *createnode(int data, int print) {
	struct node *ret = kmalloc(sizeof(struct node));
	if (print == 1)
		printf("%p   ", ret);
	ret->data = data;
	ret->next = NULL;
	return ret;
}
void traverse_linked_list() {
	struct node *head = createnode(5, 0);
//	printf("head: %p\n", head);
	int i = 0;
	struct node *current, *new;
	current = head;
	while (i < 200) {

		new = createnode(i, 0);
		if (i % 10 == 0)
			printf("%p  ", new);
		current->next = new;
		current = current->next;
		i++;
	}
	current = head;
	void *test = kmalloc(4000);
	printf("test:%p", test);
	int *test2 = kmalloc(sizeof(int));
	printf("test2: %p", test2);
	int *test3 = kmalloc(sizeof(int));
	printf("test3: %p", test3);
	struct bignode *new1 = kmalloc(sizeof(struct bignode));
	new1->data1 = 1;
	new1->data2 = 2;
	new1->data3 = 3;
	new1->data4 = 4;
	new1->data5 = 5;
	new1->next = NULL;
	printf("new data:%p ", new1);
	int counter = 0;
	struct node* temp;
	current = head;
	while (counter != 3) {
		printf("deleting data: %p     ", current);
		temp = current;
		current = current->next;
		kfree(temp);
		counter++;
	}
	head = current;
	current = head;
	while (current->next != NULL) { //todo: very important to deal with this null pointer exception we need to handle
		current = current->next;
	}
	current->next = createnode(100, 1);
	current = current->next;
	printf("node: %p %d", current, current->data);
	struct node *cur = createnode(200, 1);
	printf("\ncur: %p  %d", cur, cur->data);
	struct node *cur2 = createnode(200, 1);
	printf("\ncur: %p  %d", cur2, cur2->data);
}

//Function idea taken from OSDevWiki http://wiki.osdev.org/ELF_Tutorial
//Its obvious once you know how but still referenced :p
//this function given the ELF file header gives the base location of the section headers
//Elf64_Shdr *get_elf_section_header(Elf64_Ehdr *header){
//	return (Elf64_Shdr *)((uint64_t)header + (uint64_t)(header->e_shoff));
//}
////this function given the base section headers gives the index of the section we want.
//Elf64_Shdr *get_elf_section_index(Elf64_Shdr *header, int current){
//	return &(header[current]);
//}
////this file gives the name of the section(header) it takes the parameter the base of the ELF file header
////and the offset into the string in .shstrtab
//char *get_elf_string_loc(uint64_t base_file, Elf64_Shdr *header, int offset){
//	char *ret = (char *)(base_file + (uint64_t)(header->sh_offset) + (uint64_t)offset);
//	return ret;
//}
//Elf64_Shdr *match_section_elf(Elf64_Ehdr *current_elf, char *section){
//	Elf64_Shdr *base_section = get_elf_section_header(current_elf);
//	Elf64_Shdr *name_header = get_elf_section_index(base_section, current_elf->e_shstrndx);// this gets the section which has the names of the sections
//	for(int j = 0; j<current_elf->e_shnum;j++){
//		Elf64_Shdr *current_header = get_elf_section_index(base_section,j);
//		int offset_str = (int)(current_header->sh_name);
//		char *current_str = get_elf_string_loc((uint64_t)current_elf,name_header, offset_str);
////		printf("%s  ", current_str);
//		if(strcmp(current_str, section) == 0){
//			printf("%s\n", current_str);
//			return current_header;
//		}
//	}
//	return NULL;
//}
//void find_text_elf(Elf64_Ehdr *current){
//	Elf64_Shdr *text_header = match_section_elf(current, ".text");
//	if(text_header != NULL)
//		printf("%x", text_header->sh_addr);
//}
//void find_rodata_elf(Elf64_Ehdr *current){
//	Elf64_Shdr *text_header = match_section_elf(current, ".rodata");
//	if(text_header != NULL)
//		printf("%x", text_header->sh_addr);
//}
//void find_data_elf(Elf64_Ehdr *current){
//	Elf64_Shdr *text_header = match_section_elf(current, ".data");
//	if(text_header != NULL)
//		printf("%x", text_header->sh_addr);
//}
//void find_bss_elf(Elf64_Ehdr *current){
//	Elf64_Shdr *text_header = match_section_elf(current, ".bss");
//	if(text_header != NULL)
//		printf("%x", text_header->sh_addr);
//}
//void load_executable(char *str){
//	struct posix_header_ustar *current = (struct posix_header_ustar *)&_binary_tarfs_start;
//	int i = 0;
//	while((uint64_t)current<(uint64_t)(&_binary_tarfs_end)){
//		if(strcmp(current->name,str) == 0){
//			printf("%s", current->name);
//			uint64_t next = (uint64_t)((uint64_t)current + (uint64_t)sizeof(struct posix_header_ustar));
//			Elf64_Ehdr *temp = (Elf64_Ehdr *)(next);
////			printf("header: %x %c\n",(int)(temp->e_ident[0]),temp->e_ident[1]);
////			printf("type: %d\n",temp->e_type);
////			printf("machine: %d\n",temp->e_machine);
////			printf("struct size:%d\n", temp->e_shentsize);
////			printf("number:%d\n",temp->e_shnum);
////			printf("index:%d\n",temp->e_shstrndx);
//
//			find_text_elf(temp);
//			find_rodata_elf(temp);
//			find_data_elf(temp);
//			find_bss_elf(temp);
//			break;
//		}
//		//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
//		uint64_t header_next = (uint64_t)((align(convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))+sizeof(struct posix_header_ustar) + (uint64_t)current);
//		printf("header : %x", header_next);
//		current = (struct posix_header_ustar *)(header_next);
//		i++;
//
//	}
//}
//void test_process_switch() {
//	printf("\n we going to process switch now");
//	preempt();
//	printf("\n we are back");
//}
//void stack_ring_three() {
//	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
//	tss.rsp0 = (uint64_t) (stack_kernel + 0x1000);
//	__asm__ __volatile__("movw $0x2B,%%ax\n\t"
//			"ltr %%ax"
//			:::"rax"
//	);
//
//}
void switch_user_mode() {
	__asm__ __volatile__ (
			"cli\n\t"
			"movw $0x23, %ax\n\t"
			"movw %ax, %ds\n\t"
			"movw %ax, %es\n\t"
			"movw %ax, %fs\n\t"
			"movw %ax, %gs\n\t"
			"movq %rsp, %rax\n\t"
			"pushq $0x23\n\t"
			"pushq %rax\n\t"
			"pushf\n\t"
			"popq %rax\n\t"
			"orq $0x200, %rax\n\t"
			"pushq %rax\n\t"
			"pushq $0x1B\n\t"
			"push $0x4000e8\n\t"
			"iretq\n\t"
	);

}
void test_user_function() {
	while (1)
		;
}
void pagefault_tests(){
        uint64_t * p = (uint64_t *)0x1000;
        printf("causing fault..%x ", *p);

      p = (uint64_t *)0xffffffffffffffff;
      printf("causing fault..%x ", *p);

}
void read_msr(uint32_t msr_id, uint32_t *lo, uint32_t *hi){
      __asm__ __volatile__("rdmsr":"=a"(*lo), "=d"(*hi) : "c"(msr_id));
}
void write_msr(uint32_t msr, uint32_t lo, uint32_t hi){
      __asm__ __volatile__ ("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
void test_syscall_rip(){
	__asm__ __volatile__("cli\n\t"
						 "movq %%rax, %0"
						:"=m"(current_syscall)
						:
						:"%rax","%rbx", "%rdx","%rcx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14");

	__asm__ __volatile__("movq %%rsp, %0"
						 :"=m"(rsp_user_syscall)
						 :
						 :"%rax","%rbx", "%rdx","%rcx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14");
	__asm__ __volatile__("movq %%rcx, %0"
							 :"=m"(rip_syscall)
							 :
							 :"%rax","%rbx", "%rdx","%rcx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14");

	__asm__ __volatile__("movq %0, %%rsp"
						 :
						 :"a"(currenttask->state.kernel_rsp)
						  :"%rbx", "%rdx","%rcx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14","%r15");
	//order rax ->rdi->rsi->rdx->r10->r8->r9
	uint64_t args_list;
	__asm__ __volatile__(
			"pushq %%rcx\n\t"
			"pushq %%rdi\n\t"
			"pushq %%rsi\n\t"
			"pushq %%rdx\n\t"
			"pushq %%r10\n\t"
			"pushq %%r8\n\t"
			"pushq %%r9\n\t"
			"movq %%rsp, %0"
			:"=r"(args_list)
			:
			:"%rbx", "%rdx","%rcx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14","%r15");
//	currenttask->current_syscall = current_syscall;
//	currenttask->rsp_stack_syscall = rsp_user_syscall;
	printf("\n args_list%p", args_list);

//	uint32_t syscall_number = 0;
//	__asm__ __volatile__(
//						  "movq %%rcx, %0"
//						 :"=m"(syscall_number)
//						 :
//						 :"%rax","%rbx","%rcx","%rdx","%rdi", "%rsi", "%rbp", "%r8","%r9","%r10","%r11","%r12","%r13","%r14","%r15");
	uint64_t retvalue;
	if(current_syscall == 60){
		temp_preempt_exit(currenttask->state.kernel_rsp);
		uint64_t kernel_stack = (uint64_t)(currenttask->state.kernel_rsp);

		__asm__ __volatile__("movq %0, %%rsp"
							:
							:"m"(kernel_stack));
		__asm__ __volatile__("popq %rax\n\t"
							 "movq %rax, %gs\n\t"
							 "popq %rax\n\t"
							 "movq %rax, %fs\n\t"
							 "popq %rax\n\t"
							 "movq %rax, %es\n\t"
							 "popq %rax\n\t"
							 "movq %rax, %ds\n\t"
							 "popq %r15\n\t"
							 "popq %r14\n\t"
							 "popq %r13\n\t"
							 "popq %r12\n\t"
							 "popq %r11\n\t"
							 "popq %r10\n\t"
							 "popq %r9\n\t"
							 "popq %r8\n\t"
							 "popq %rbp\n\t"
							 "popq %rsi\n\t"
							 "popq %rdi\n\t"
							 "popq %rdx\n\t"
							 "popq %rcx\n\t"
							 "popq %rbx\n\t"
							 "popq %rax\n\t"
							 "sti\n\t"
							 "iretq");
	}
	else if(current_syscall == 40){

		printf("\ninside system call");
		retvalue = 32;
	}
//	__asm__ __volatile__(
//			"movq 48(%0), %rcx "
//			:
//			:"a"(args_list)
//			:);
//	uint64_t return_stack = currenttask->rsp_stack_syscall;
	__asm__ __volatile__("movq %0,%%rsp\n\t"
						 :
						 :"m"(rsp_user_syscall), "a"(retvalue), "c"(rip_syscall)
						 :);
	__asm__ __volatile__("movq $72, %rsp\n\t"
						 "sti\n\t"
						 "sysretq");
}


void enable_syscall(){
      //stack pointers for syscall/sysret is not specified through MSR
      //need to program RFLAGS but syscall/sysret saves/restores RFLAGS
      //RFLAGS is stored in R11 and RIP in RCX
      uint32_t msr_id = 0xC0000080;
      uint32_t lo, hi;
      read_msr(msr_id, &lo, &hi);
//      printf("MSR: %x %x\n",hi, lo);
      lo |= 1;
      write_msr(msr_id, lo, hi);
//      read_msr(msr_id, &lo, &hi);
//      printf("0xC0000080: %x\n",hi, lo);
      uint64_t STAR = 0; //Target code segment — Reads a non-NULL selector from IA32_STAR[47:32].
      uint64_t kern_code = 0x0010;
      uint64_t user_code = 0x0023;
      STAR |= kern_code<<32;
      STAR |= user_code<<48;
//      printf("STAR: %p",STAR);
      uint64_t main1 = (uint64_t)(test_syscall_rip);
      write_msr(0xC0000081, (uint32_t)(STAR), (uint32_t)(STAR>>32));
      write_msr(0xC0000082,(uint32_t)(main1),(uint64_t)(main1>>32));
      read_msr(0xC0000081, &lo, &hi);
//            printf("MSR: %x %x\n",hi, lo);
            read_msr(0xC0000082, &lo, &hi);
//                        printf("MSR: %x %x\n",hi, lo);
}
void test_free_pages(){
	void *virtual_addr = kmalloc(64);
	uint64_t * target_pte = virtual_addr_pte((uint64_t)virtual_addr);
	uint64_t page_phys_addr  = (uint64_t)next_entity_base(target_pte);
	printf("%p", page_phys_addr);
	int count = get_ref_count(page_phys_addr);
	printf("free: %d  ", count);
	increase_ref_count(page_phys_addr);
	count = get_ref_count(page_phys_addr);
		printf("free: %d  ", count);
		decrease_ref_count(page_phys_addr);
		count = get_ref_count(page_phys_addr);
		printf("free: %d  ", count);
		decrease_ref_count(page_phys_addr);
				count = get_ref_count(page_phys_addr);
				printf("free: %d  ", count);
				virtual_addr = kmalloc(64);
					target_pte = virtual_addr_pte((uint64_t)virtual_addr);
					page_phys_addr = 0;
					page_phys_addr  = (uint64_t)next_entity_base(target_pte);
					printf("%p", page_phys_addr);
}

void start(uint32_t* modulep, void* physbase, void* physfree) {
	while (modulep[0] != 0x9001) {
		modulep += modulep[1] + 2;
	};
//	printf("physbase:physfree\n %p %p ", physbase, physfree);
	for (smap = (struct smap_t*) (modulep + 2);
			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
			++smap) {
		if (smap->type == 1 /* memory */&& smap->length != 0) {
//			printf("Length of memory:%d\n", smap->length);
//			printf("Available Physical Memory [%x-%x]\n", smap->base,
//					smap->base + smap->length);
		}
	}
//	printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);

	//	printf("%x", pml_base_ptr_process);

	// kernel starts here
	manage_memory(physbase, physfree, modulep);
//	pagingTests(physbase, physfree, modulep);
	/*	should be invoked without manage_memory above.
	 and do_padding in it shud not be done.
	 pagingTests(physbase, physfree, modulep);
	 */

//	process_stuff(); // <<<<<<------------- works fine here


init_caches();
//enable_syscall();
//	traverse_linked_list();
init_init_IDT();
config_PIC();
add_custom_interrupt();
init_keyboard_map();
keyboard_init();
__asm__ __volatile("cli");
//test_free_pages();
init_tarfs();
//find_file_tarfs("bin/hello");
//int fd = open_tarfs("test/test.txt", O_RDONLY);
//printf("\nfd:%d ", fd);
//char *buffer = kmalloc(100);//change this to malloc when we implement brk
//read_tarfs(fd, buffer, 100);
//printf("fff%s", buffer);
//char *buff = "this is a write \nsystem call";
//int ret = write_system_call(1, buff, 1000);
//printf("%d\n", ret);
//	process_stuff(); // <<<<<<------------- here it faults, but not before init_caches()
//	stack_ring_three();
//	switch_user_mode();
//	_jump_usermode();

kernel_process_init();
//		test_process_switch();


//	printf("\n presence: %d", is_linear_addr_mapped(0x4000))
//	load_executable("bin/hello");

//	pagefault_tests();
//	load_elf_trial();
//		uint64_t *temp = get_physical_pml4_base_for_process();
//		update_cr3((uint64_t *)(temp));
//		uint64_t *temp = get_physical_pml4_base_for_process();
//						update_cr3((uint64_t *)(temp));
//					uint64_t *p = (uint64_t *)0x400000;
//					uint64_t *process_physical = (uint64_t *)get_free_frame();
//					setup_kernel_page_tables((uint64_t) p,
//									(uint64_t) process_physical);
//					*p = 23;
//					printf("fsfsdf %d",(int)(*p));
		__asm__ __volatile__("sti");
//	 cause page fault
//	uint64_t * p = (uint64_t *) 0x1000;
//	printf(" causing fault..%x ", *p);
//	printf(" alive after fault %x", *p);

//	load_elf_trial();

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
__asm__ __volatile__("cli");
start(
		(uint32_t*) ((char*) (uint64_t) loader_stack[3]
				+ (uint64_t) &kernmem - (uint64_t) &physbase), &physbase,
		(void*) (uint64_t) loader_stack[4]);
while (1)
	;
}
