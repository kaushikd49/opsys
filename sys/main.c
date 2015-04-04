#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h>
#include <sys/paging.h>
#include <sys/main.h>
#include <sys/isr_stuff.h>
#include <sys/initkeyboard.h>
#include <sys/printtime.h>
#include<sys/kmalloc.h>

#define INITIAL_STACK_SIZE 4096

extern char video_buffer[4096];
struct smap_t {
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed)) *smap;
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;
struct node{
	int data;
	struct node *next;
};
struct bignode{
	long data1;
	long data2;
	long data3;
	long data4;
	long data5;
	struct bignode *next;
};
struct node *createnode(int data,int print){
	struct node *ret = kmalloc(sizeof(struct node));
	if(print == 1)
		printf("%p   ", ret);
	ret->data = data;
	ret->next = NULL;
	return ret;
}
void traverse_linked_list(){
	struct node *head = createnode(5,0);
//	printf("head: %p\n", head);
	int i = 0;
	struct node *current, *new;
	current = head;
	while(i<200){

		new = createnode(i,0);
		if(i %10 == 0)
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
	int counter =0;
	struct node* temp;
	current = head;
	while(counter!=3){
		printf("deleting data: %p     ",current);
		temp = current;
		current = current->next;
		kfree(temp);
		counter++;
	}
	head = current;
	current = head;
	while(current->next!=NULL){//todo: very important to deal with this null pointer exception we need to handle
		current = current->next;
	}
	current->next = createnode(100,1);
	current = current->next;
	printf("node: %p %d", current, current->data);
	struct node *cur = createnode(200,1);
		printf("\ncur: %p  %d", cur,cur->data);
		struct node *cur2 = createnode(200,1);
				printf("\ncur: %p  %d", cur2,cur2->data);
}
uint64_t convert_ocatalstr_todecimal(char octal[10]){
	int i = 0;
	uint64_t number = 0;
	char present;
	int digit;
	while(octal[i] != '\0'){
		present = octal[i];
		digit = (int)((int)(present) - (int)('0'));
		number = number*8 + digit;
		i++;
	}
	return number;
}
int strcmp(char *string1, char *string2) {
	uint64_t len = 0;
	while (string1[len] != '\0' && string2[len] != '\0') {
		if (string1[len] != string2[len])
			break;
		len++;
	}
	if (string1[len] == string2[len])
		return 0;
	else if (string1[len] == '\0'
			|| (string2[len] != '\0' && string1[len] < string2[len])) {
		return (int) string2[len];
	} else {
		return (int) string1[len];
	}

}
//Function idea taken from OSDevWiki http://wiki.osdev.org/ELF_Tutorial
//Its obvious once you know how but still referenced :p
//this function given the ELF file header gives the base location of the section headers
Elf64_Shdr *get_elf_section_header(Elf64_Ehdr *header){
	return (Elf64_Shdr *)((uint64_t)header + (uint64_t)(header->e_shoff));
}
//this function given the base section headers gives the index of the section we want.
Elf64_Shdr *get_elf_section_index(Elf64_Shdr *header, int current){
	return &(header[current]);
}
//this file gives the name of the section(header) it takes the parameter the base of the ELF file header
//and the offset into the string in .shstrtab
char *get_elf_string_loc(uint64_t base_file, Elf64_Shdr *header, int offset){
	char *ret = (char *)(base_file + (uint64_t)(header->sh_offset) + (uint64_t)offset);
	return ret;
}
void load_elf_trial(){
	struct posix_header_ustar *current = (struct posix_header_ustar *)&_binary_tarfs_start;
	printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	//current = (struct posix_header_ustar *)((uint64_t)current + (uint64_t)sizeof(struct posix_header_ustar));
	//printf("%s",current->size	);
	int i = 0;
	while((uint64_t)current<(uint64_t)(&_binary_tarfs_end)){
	printf("  %s  %d %s\n", current->name, convert_ocatalstr_todecimal(current->size),current->prefix);
	if(strcmp(current->name,"bin/hello") == 0){
		Elf64_Ehdr *temp = (Elf64_Ehdr *)((uint64_t)current + (uint64_t)sizeof(struct posix_header_ustar));
//		printf("header: %x %c\n",(int)(temp->e_ident[0]),temp->e_ident[1]);
//		printf("type: %d\n",temp->e_type);
//		printf("machine: %d\n",temp->e_machine);
//		printf("struct size:%d\n", temp->e_shentsize);
//		printf("number:%d\n",temp->e_shnum);
//		printf("index:%d\n",temp->e_shstrndx);
		Elf64_Shdr *base_section = get_elf_section_header(temp);
		Elf64_Shdr *name_header = get_elf_section_index(base_section, temp->e_shstrndx);
		for(int j = 0; j<temp->e_shnum;j++){
			Elf64_Shdr *current_header = get_elf_section_index(base_section,j);
			int offset_str = (int)(current_header->sh_name);
			char *current = get_elf_string_loc((uint64_t)temp,name_header, offset_str);
			printf("%s   ", current);
		}
		break;
	}
//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
	current = (struct posix_header_ustar *)(convert_ocatalstr_todecimal(current->size)+sizeof(struct posix_header_ustar) + (uint64_t)current);
	i++;
	}

}
void start(uint32_t* modulep, void* physbase, void* physfree) {
	while (modulep[0] != 0x9001) {
		modulep += modulep[1] + 2;
	};
	printf("physbase:physfree\n %p %p ",physbase, physfree);
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

	//	printf("%x", pml_base_ptr_process);

	// kernel starts here
	manage_memory(physbase, physfree, modulep);
//	pagingTests(physbase, physfree, modulep);
	/*	should be invoked without manage_memory above.
		todo: this is failing as freelist is returning 0x0 as the free page
		pagingTests(physbase, physfree, modulep);
	*/

	init_caches();
	//traverse_linked_list();
	init_init_IDT();
	config_PIC();
	add_custom_interrupt();
	init_keyboard_map();
	keyboard_init();

	// cause page fault
//	uint64_t * p = (uint64_t *)0x1000;
//	printf("causing fault..%x ", *p);
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
