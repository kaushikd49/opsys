#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/pagingglobals.h>
#include <sys/freelist.h>
#include <sys/paging.h>
#include<sys/kmalloc.h>
#include<sys/tarfs.h>
#include<sys/process.h>
#define VM_READ 1<<0
#define VM_WRITE 1<<1
#define VM_EXEX 1<<2
#define USER_CODE (3<<3 | 3)
#define USER_DATA (4<<3 | 3)
static task_struct_t *currenttask;
static task_struct_t *lasttask;
static task_struct_t taskone;
typedef struct elf_section_info{
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
}elf_sec_info_t;
/*
 * elf reference functions
 */
extern void process_switch(process_state *, process_state *);
extern void process_switch_user(process_state *, process_state *);
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
Elf64_Shdr *match_section_elf(Elf64_Ehdr *current_elf, char *section){
	Elf64_Shdr *base_section = get_elf_section_header(current_elf);
	Elf64_Shdr *name_header = get_elf_section_index(base_section, current_elf->e_shstrndx);// this gets the section which has the names of the sections
	for(int j = 0; j<current_elf->e_shnum;j++){
		Elf64_Shdr *current_header = get_elf_section_index(base_section,j);
		int offset_str = (int)(current_header->sh_name);
		char *current_str = get_elf_string_loc((uint64_t)current_elf,name_header, offset_str);
//		printf("%s  ", current_str);
		if(strcmp(current_str, section) == 0){
			printf("%s\n", current_str);
			return current_header;
		}
	}
	return NULL;
}
elf_sec_info_t *find_text_elf(Elf64_Ehdr *current){
	Elf64_Shdr *text_header = match_section_elf(current, ".text");

	if(text_header != NULL){
		elf_sec_info_t *text_info = kmalloc(sizeof(elf_sec_info_t));
		text_info->sh_addr = text_header->sh_addr;
		text_info->sh_offset = text_header->sh_offset;
		text_info->sh_size = text_header->sh_size;
		return text_info;
	}
	return NULL;
}
elf_sec_info_t *find_rodata_elf(Elf64_Ehdr *current){
	Elf64_Shdr *rodata_header = match_section_elf(current, ".rodata");
	if(rodata_header != NULL){
		elf_sec_info_t *rodata_info = kmalloc(sizeof(elf_sec_info_t));
		rodata_info->sh_addr = rodata_header->sh_addr;
		rodata_info->sh_offset = rodata_header->sh_offset;
		rodata_info->sh_size = rodata_header->sh_size;
		return rodata_info;
	}
	return NULL;
}
elf_sec_info_t *find_data_elf(Elf64_Ehdr *current){
	Elf64_Shdr *data_header = match_section_elf(current, ".data");
	if(data_header != NULL){
			elf_sec_info_t *data_info = kmalloc(sizeof(elf_sec_info_t));
			data_info->sh_addr = data_header->sh_addr;
			data_info->sh_offset = data_header->sh_offset;
			data_info->sh_size = data_header->sh_size;
			return data_info;
		}
	return NULL;
}
elf_sec_info_t *find_bss_elf(Elf64_Ehdr *current){
	Elf64_Shdr *bss_header = match_section_elf(current, ".bss");
	if(bss_header != NULL){
		elf_sec_info_t *bss_info = kmalloc(sizeof(elf_sec_info_t));
		bss_info->sh_addr = bss_header->sh_addr;
		bss_info->sh_offset = bss_header->sh_offset;
		bss_info->sh_size = bss_header->sh_size;
		return bss_info;
	}
	return NULL;
}
void elf_mem_copy(char *virtual_addr, char *elf_addr, uint64_t size){
	char *current = virtual_addr;
	char *elf_current = elf_addr;
	char *limit = (char *)((uint64_t)virtual_addr + (uint64_t)size);
	while(current < limit){
		if(!(is_linear_addr_mapped((uint64_t)current))){
			void *free_frame = (void *)get_free_frames(0);
			setup_process_page_tables((uint64_t)current, (uint64_t)free_frame);
		}
		*current = *elf_current;
		current++;
		elf_current++;
	}
}
void elf_zerod_copy(char *virtual_addr, uint64_t size){
	char *current = virtual_addr;
	char *limit = (char *)((uint64_t)virtual_addr + (uint64_t)size);
	while(current < limit){
		if(!(is_linear_addr_mapped((uint64_t)current))){
			void *free_frame = (void *)get_free_frames(0);
			setup_process_page_tables((uint64_t)current, (uint64_t)free_frame);
			*current = 0;
		}

		current++;
	}
}
//when processes are ready, I would like to make this a procedure where given the mem_desc I can load the executable into the memstruct
void load_executable(char *str){
	struct posix_header_ustar *current = (struct posix_header_ustar *)&_binary_tarfs_start;
	int i = 0;
	elf_sec_info_t *text_info = NULL;
	elf_sec_info_t *rodata_info = NULL;
	elf_sec_info_t *data_info = NULL;
	elf_sec_info_t *bss_info = NULL;
	Elf64_Ehdr *temp = NULL;
	while((uint64_t)current<(uint64_t)(&_binary_tarfs_end)){
		if(strcmp(current->name,str) == 0){
			printf("%s", current->name);
			uint64_t next = (uint64_t)((uint64_t)current + (uint64_t)sizeof(struct posix_header_ustar));
			temp = (Elf64_Ehdr *)(next);
			text_info = find_text_elf(temp);
			rodata_info = find_rodata_elf(temp);
			data_info = find_data_elf(temp);
			bss_info = find_bss_elf(temp);
			break;
		}
		//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
		uint64_t header_next = (uint64_t)((align(convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))+sizeof(struct posix_header_ustar) + (uint64_t)current);
		printf("header : %x", header_next);
		current = (struct posix_header_ustar *)(header_next);
		i++;

	}
	uint64_t section_offset;
	if(text_info!=NULL){
		section_offset = (uint64_t)temp + (uint64_t)text_info->sh_offset;
		printf("text:  %x  %x  %x\n",text_info->sh_addr, section_offset, text_info->sh_size );
		elf_mem_copy((char *)(text_info->sh_addr), (char *)section_offset, (text_info->sh_size));
	}
	if(rodata_info!=NULL){
		section_offset = (uint64_t)temp + (uint64_t)rodata_info->sh_offset;
		printf("rodata:  %x  %x  %x\n",rodata_info->sh_addr, section_offset, rodata_info->sh_size );
		elf_mem_copy((char *)(rodata_info->sh_addr), (char *)section_offset,(rodata_info->sh_size));
	}
	if(data_info!=NULL){
		section_offset = (uint64_t)temp + (uint64_t)data_info->sh_offset;
		printf("data:  %x  %x  %x\n",data_info->sh_addr, section_offset, data_info->sh_size );
		elf_mem_copy((char *)(data_info->sh_addr), (char *)section_offset, data_info->sh_size);
	}
	if(bss_info != NULL){
		section_offset = (uint64_t)temp + (uint64_t)data_info->sh_offset;
		printf("bss:  %x  %x  %x\n",bss_info->sh_addr, section_offset, bss_info->sh_size );
		elf_zerod_copy((char *)(bss_info->sh_addr), data_info->sh_size);
	}

	kfree(text_info);
	kfree(rodata_info);
	kfree(data_info);
	kfree(bss_info);

	currenttask->state.rip = (uint64_t)(temp->e_entry);
	uint64_t stack_page = 0x7000000;
	void *free_frame = (void *)get_free_frames(0);
	setup_process_page_tables((uint64_t)stack_page, (uint64_t)free_frame);
	currenttask->state.rsp = (uint64_t)stack_page + 0x1000;
	//add code to initialize stack
	//heap
}
/*
 *
 */
/*
 * PROCESS Global variables section
 *
 */
struct task_struct *init_task = NULL;
static uint64_t pid_counter = 1;
/*
 * end of PROCESS Global Variables
 */
uint64_t get_next_pid(){
	uint64_t pid_value = pid_counter;
	pid_counter += 1;//assumption we never go above max value of uint64_t reasonable assumption
	return pid_value;
}
void init_text_vm(){

}
void init_vm(struct mem_desc *mem_map){
//	create
}
void map_process_vm(task_struct_t *task){
	task->mem_map = kmalloc(sizeof(struct mem_desc));
	init_vm(task->mem_map);
}
//todo:initialize process state function init_process_state



/*
 * rememver:
 * Kernel space is flagged in the page tables as exclusive to privileged code (ring 2 or lower), hence a page fault is triggered
 * if user-mode programs try to touch it
 */

void process_init(){

}
void maintasktwo(){
	printf("\nthis is main task two ");
	preempt();
}
//this needs to be referenced for sure idea taken from here
//idea taken from here: http://wiki.osdev.org/Kernel_Multitasking
void create_process(char *executable, uint64_t ppid){
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	task_struct_t *temp_start = currenttask->next;
	task_struct_t *parent_task = NULL;
	while(temp_start != currenttask && temp_start->pid != ppid ){
		temp_start = temp_start->next;
	}
	//assumption here is that the process has a parent, then the parent is found, have to handle zombie process somehow need to know how.
	parent_task = temp_start;
	//add the process to the end of list
	if(parent_task->next == parent_task){
		parent_task->next = task;
		task->next = parent_task;
		lasttask = task;
	}
	else{
		task->next = lasttask->next;
		lasttask->next = task;
		lasttask = task;}
	kernel_create_process(task, parent_task,executable);
}
void kernel_process_init(){
	//this function just stores the current cr3 as the processes page table since we are doing kernel preemption that is fine. For user process this will be a bit involved
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
						 "movq %%rax, %0\n\t"
			   	   	     :"=m"(taskone.state.cr3)
						 :
						 :"%rax");
	__asm__ __volatile__("pushfq\n\t"
			 	 	 	 "movq (%%rsp),%%rax\n\t"
						 "movq %%rax, %0\n\t"
						 "popfq\n\t"
						 :"=m"(taskone.state.flags)
						 :
						 :"%rax");
	//creating a secon process and that process has the same globals as the init process
	currenttask = &taskone;
	currenttask->next = currenttask;
	currenttask->pid = 1;
	create_process("bin/hello", 1);
	create_process("bin/hello2", 1);

}
char *strcpy(char *dst, char *src) {
	uint64_t len = 0;
	while (src[len] != '\0') {
		dst[len] = src[len];
		len++;
	}
	dst[len] = '\0';
	return dst;
}
void kernel_create_process(task_struct_t *task, task_struct_t *parent_task, char *executable){
	//using the user level process struct it is fine for now
	task->mem_map =NULL;
	task->pid = get_next_pid();
	task->ppid = parent_task->pid;//this need to be more involved.
	task->state.cs = USER_CODE; // for a user level process these 4 registers will matter but we will worry when we get there
	task->state.ds = USER_DATA;
	task->state.es = USER_DATA;
	task->state.ss = USER_DATA;
	task->state.gs = USER_DATA;
	task->state.fs = USER_DATA;
	task->state.r10 = 0;
	task->state.r11 = 0;
	task->state.r12 = 0;
	task->state.r13 = 0;
	task->state.r14 = 0;
	task->state.r15 = 0;
	task->state.r8 = 0;
	task->state.r9 = 0;
	task->state.rax = 0;
	task->state.rbx = 0;
	task->state.rcx = 0;
	task->state.rdx = 0;
//	task->state.rip = (uint64_t) main;
	task->state.rbp = 0;
	task->state.rdi = 0;
	task->state.rsi = 0;

	task->state.flags = parent_task->state.flags;
	// need to assign a new stack and since it grows down, we need to change taht to the end of the page too.
	strcpy((*task).executable,executable);
}

void preempt(){
	printf("\nswitching");
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
	//adding the load of executable here, it should not be here..
	uint64_t *temp = get_physical_pml4_base_for_process();

	update_cr3(temp);
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
							 "movq %%rax, %0\n\t"
				   	   	     :"=m"(currenttask->state.cr3)
							 :
							 :"%rax");

	load_executable(currenttask->executable);
	process_switch_user(&last->state, &currenttask->state);
}
