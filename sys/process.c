#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/pagingglobals.h>
#include <sys/freelist.h>
#include <sys/paging.h>
#include <sys/kmalloc.h>
#include <sys/tarfs.h>
#include <sys/process.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/tarfs_FS.h>
#include <sys/kernel_thread.h>
#include <sys/scheduling.h>
#include <sys/isr_stuff.h>
#include <sys/nanosleep_functions.h>
#include <errno.h>
#include <sys/cleanup.h>

#define THIRTY_TWO_PAGES 0x32000
#define ENV_START 0x7002000
#define ENV_END 0x701ffff
#define ENV_TEMP_START 0x7022000
#define ENV_TEMP_END 0x703ffff
#define STACK_TEMP_START 0x7040000
#define STACK_TEMP_END 0x705ffff
#define VM_READ 1<<0
#define VM_WRITE 1<<1
#define VM_EXEX 1<<2
uint64_t *ENV_SWAP_START;
uint64_t *STACK_SWAP_START;
uint64_t limit = 1 << 30;
extern int seconds_boot;
extern int ms_boot;
//static task_struct_t *currenttask; //todo: initialize
static task_struct_t *lasttask;
static task_struct_t taskone;
extern uint64_t BASE_CURSOR_POS;
uint64_t *global_keyboard;
typedef struct elf_section_info {
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
} elf_sec_info_t;
struct execucatable_t {
	elf_sec_info_t *text_info;
	elf_sec_info_t *rodata_info;
	elf_sec_info_t *data_info;
	elf_sec_info_t *bss_info;
	//===================================
	elf_sec_info_t *ehframe_info;
	elf_sec_info_t *got_info;
	elf_sec_info_t *gotplt_info;
	Elf64_Ehdr *temp;
	int is_script;
};

typedef struct execucatable_t executable_t;
uint64_t setup_new_process(char *binary, char *argv[], char *envp[],
		executable_t *executable);

/*
 * elf reference functions
 */
extern void process_switch(process_state *, process_state *);
extern void process_switch_user(process_state *, process_state *);
extern void process_switch_cooperative(process_state *, process_state *,
		uint64_t);
uint64_t convert_ocatalstr_todecimal(char octal[10]) {
	int i = 0;
	uint64_t number = 0;
	char present;
	int digit;
	while (octal[i] != '\0') {
		present = octal[i];
		digit = (int) ((int) (present) - (int) ('0'));
		number = number * 8 + digit;
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

void copy_string(char *current, char **current_environ) {

	while (*current != '\0') {
		**current_environ = *current;
		*current_environ = *current_environ + 1;
		current++;
	}
	**current_environ = '\0';
	*current_environ = *current_environ + 1;

}
//Function idea taken from OSDevWiki http://wiki.osdev.org/ELF_Tutorial
//Its obvious once you know how but still referenced :p
//this function given the ELF file header gives the base location of the section headers

Elf64_Shdr *get_elf_section_header(Elf64_Ehdr *header) {
	return (Elf64_Shdr *) ((uint64_t) header + (uint64_t) (header->e_shoff));
}
//this function given the base section headers gives the index of the section we want.
Elf64_Shdr *get_elf_section_index(Elf64_Shdr *header, int current) {
	return &(header[current]);
}
//this file gives the name of the section(header) it takes the parameter the base of the ELF file header
//and the offset into the string in .shstrtab
char *get_elf_string_loc(uint64_t base_file, Elf64_Shdr *header, int offset) {
	char *ret = (char *) (base_file + (uint64_t) (header->sh_offset)
			+ (uint64_t) offset);
	return ret;
}
Elf64_Shdr *match_section_elf(Elf64_Ehdr *current_elf, char *section) {
	Elf64_Shdr *base_section = get_elf_section_header(current_elf);
	Elf64_Shdr *name_header = get_elf_section_index(base_section,
			current_elf->e_shstrndx); // this gets the section which has the names of the sections
	for (int j = 0; j < current_elf->e_shnum; j++) {
		Elf64_Shdr *current_header = get_elf_section_index(base_section, j);
		int offset_str = (int) (current_header->sh_name);
		char *current_str = get_elf_string_loc((uint64_t) current_elf,
				name_header, offset_str);
//		printf("%s  ", current_str);
		if (strcmp(current_str, section) == 0) {
//			printf("%s\n", current_str);
			return current_header;
		}
	}
	return NULL;
}
elf_sec_info_t *find_text_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *text_header = match_section_elf(current, ".text");

	if (text_header != NULL) {
		elf_sec_info_t *text_info = kmalloc(sizeof(elf_sec_info_t));
		text_info->sh_addr = text_header->sh_addr;
		text_info->sh_offset = text_header->sh_offset;
		text_info->sh_size = text_header->sh_size;
		return text_info;
	}
	return NULL;
}
elf_sec_info_t *find_rodata_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *rodata_header = match_section_elf(current, ".rodata");
	if (rodata_header != NULL) {
		elf_sec_info_t *rodata_info = kmalloc(sizeof(elf_sec_info_t));
		rodata_info->sh_addr = rodata_header->sh_addr;
		rodata_info->sh_offset = rodata_header->sh_offset;
		rodata_info->sh_size = rodata_header->sh_size;
		return rodata_info;
	}
	return NULL;
}
elf_sec_info_t *find_data_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *data_header = match_section_elf(current, ".data");
	if (data_header != NULL) {
		elf_sec_info_t *data_info = kmalloc(sizeof(elf_sec_info_t));
		data_info->sh_addr = data_header->sh_addr;
		data_info->sh_offset = data_header->sh_offset;
		data_info->sh_size = data_header->sh_size;
		return data_info;
	}
	return NULL;
}
elf_sec_info_t *find_bss_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *bss_header = match_section_elf(current, ".bss");
	if (bss_header != NULL) {
		elf_sec_info_t *bss_info = kmalloc(sizeof(elf_sec_info_t));
		bss_info->sh_addr = bss_header->sh_addr;
		bss_info->sh_offset = bss_header->sh_offset;
		bss_info->sh_size = bss_header->sh_size;
		return bss_info;
	}
	return NULL;
}
//================================================================
elf_sec_info_t *find_ehframe_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *ehframe_header = match_section_elf(current, ".eh_frame");
	if (ehframe_header != NULL) {
		elf_sec_info_t *ehframe_info = kmalloc(sizeof(elf_sec_info_t));
		ehframe_info->sh_addr = ehframe_header->sh_addr;
		ehframe_info->sh_offset = ehframe_header->sh_offset;
		ehframe_info->sh_size = ehframe_header->sh_size;
		return ehframe_info;
	}
	return NULL;
}
elf_sec_info_t *find_got_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *got_header = match_section_elf(current, ".got");
	if (got_header != NULL) {
		elf_sec_info_t *got_info = kmalloc(sizeof(elf_sec_info_t));
		got_info->sh_addr = got_header->sh_addr;
		got_info->sh_offset = got_header->sh_offset;
		got_info->sh_size = got_header->sh_size;
		return got_info;
	}
	return NULL;
}
elf_sec_info_t *find_gotplt_elf(Elf64_Ehdr *current) {
	Elf64_Shdr *gotplt_header = match_section_elf(current, ".got.plt");
	if (gotplt_header != NULL) {
		elf_sec_info_t *gotplt_info = kmalloc(sizeof(elf_sec_info_t));
		gotplt_info->sh_addr = gotplt_header->sh_addr;
		gotplt_info->sh_offset = gotplt_header->sh_offset;
		gotplt_info->sh_size = gotplt_header->sh_size;
		return gotplt_info;
	}
	return NULL;
}
//===================================================================
void copy_from_elf(char* current, char* limit, char* elf_current) {
	while (current < limit) {
		if (!(is_linear_addr_mapped((uint64_t) current))) {
			void* free_frame = (void*) get_free_frames(0);
			setup_process_page_tables((uint64_t) current,
					(uint64_t) free_frame);
		}
		*current = *elf_current;
		current++;
		elf_current++;
	}
}

void elf_mem_copy(char *virtual_addr, char *elf_addr, uint64_t size) {
	char *current = virtual_addr;
	char *elf_current = elf_addr;
	char *limit = (char *) ((uint64_t) virtual_addr + (uint64_t) size);
	copy_from_elf(current, limit, elf_current);
}

void copy_zero_for_range(char* current, char* limit) {
	while (current < limit) {
		if (!(is_linear_addr_mapped((uint64_t) current))) {
			void* free_frame = (void*) get_free_frames(0);
			setup_process_page_tables((uint64_t) current,
					(uint64_t) free_frame);
			*current = 0;
		}
		current++;
	}
}

void elf_zerod_copy(char *virtual_addr, uint64_t size) {
	char *current = virtual_addr;
	char *limit = (char *) ((uint64_t) virtual_addr + (uint64_t) size);
	copy_zero_for_range(current, limit);
}

void add_vma(uint64_t vma_start, uint64_t vma_end, int type,
		mem_desc_t* mem_desc_ptr) {
	mem_desc_ptr->num_vma += 1;
	vma_t* vma_ptr = kmalloc(sizeof(struct vma));

	vma_ptr->my_mem_desc = mem_desc_ptr;
	vma_ptr->vma_next = NULL;
	vma_ptr->vma_start = vma_start;
	vma_ptr->vma_end = vma_end;
	vma_ptr->type = type;

	if (mem_desc_ptr->vma_list == NULL) {
		mem_desc_ptr->vma_list = vma_ptr;
	} else {
		vma_t* temp = mem_desc_ptr->vma_list;
		while (temp->vma_next != NULL)
			temp = temp->vma_next;
		temp->vma_next = vma_ptr;
	}
}

void load_from_elf(task_struct_t *task, elf_sec_info_t* text_info,
		Elf64_Ehdr* temp, elf_sec_info_t* rodata_info,
		elf_sec_info_t* data_info, elf_sec_info_t* bss_info,
		elf_sec_info_t *ehframe_info, elf_sec_info_t *got_info,
		elf_sec_info_t *gotplt_info) {
	uint64_t section_offset;

	mem_desc_t * mem_desc_ptr = kmalloc(sizeof(struct mem_desc));
	mem_desc_ptr->num_vma = 0;
	mem_desc_ptr->vma_list = NULL;
	task->mem_map = mem_desc_ptr;

	if (text_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) text_info->sh_offset;
		mem_desc_ptr->text_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) text_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) text_info->sh_size;
		add_vma(vma_start, vma_end, 0, mem_desc_ptr);
		//		printf("text:  %x  %x  %x\n",text_info->sh_addr, section_offset, text_info->sh_size );
//		elf_mem_copy((char*) (text_info->sh_addr), (char*) section_offset,
//				(text_info->sh_size));
	}
	if (rodata_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) rodata_info->sh_offset;
		mem_desc_ptr->rodata_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) rodata_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) rodata_info->sh_size;
		add_vma(vma_start, vma_end, 1, mem_desc_ptr);

		//		printf("rodata:  %x  %x  %x\n",rodata_info->sh_addr, section_offset, rodata_info->sh_size );
//		elf_mem_copy((char*) (rodata_info->sh_addr), (char*) section_offset,
//				(rodata_info->sh_size));
	}
	if (data_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) data_info->sh_offset;
		mem_desc_ptr->data_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) data_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) data_info->sh_size;
		add_vma(vma_start, vma_end, 2, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",data_info->sh_addr, section_offset, data_info->sh_size );
		//		elf_mem_copy((char*) (data_info->sh_addr), (char*) section_offset,
		//				data_info->sh_size);
	}
	if (bss_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) bss_info->sh_offset;
		uint64_t vma_start = (uint64_t) bss_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) bss_info->sh_size;
		add_vma(vma_start, vma_end, 3, mem_desc_ptr);

		uint64_t heap_start = ((((uint64_t) vma_end) & (~(PAGE_SIZE - 1)))
				+ (PAGE_SIZE));
		mem_desc_ptr->brk = heap_start;
		add_vma(heap_start, heap_start, 5, mem_desc_ptr);
		//		printf("bss:  %x  %x  %x\n",bss_info->sh_addr, section_offset, bss_info->sh_size );
		//		elf_zerod_copy((char*) (bss_info->sh_addr), data_info->sh_size);
	}
	//================================================================
	if (ehframe_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) ehframe_info->sh_offset;
		mem_desc_ptr->ehframe_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) ehframe_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) ehframe_info->sh_size;
		add_vma(vma_start, vma_end, 6, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",ehframe_info->sh_addr, section_offset, ehframe_info->sh_size );
		//		elf_mem_copy((char*) (ehframe_info->sh_addr), (char*) section_offset,
		//				ehframe_info->sh_size);
	}
	if (got_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) got_info->sh_offset;
		mem_desc_ptr->got_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) got_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) got_info->sh_size;
		add_vma(vma_start, vma_end, 7, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",got_info->sh_addr, section_offset, got_info->sh_size );
		//		elf_mem_copy((char*) (got_info->sh_addr), (char*) section_offset,
		//				got_info->sh_size);
	}
	if (gotplt_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) gotplt_info->sh_offset;
		mem_desc_ptr->gotplt_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) gotplt_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) gotplt_info->sh_size;
		add_vma(vma_start, vma_end, 8, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",gotplt_info->sh_addr, section_offset, gotplt_info->sh_size );
		//		elf_mem_copy((char*) (gotplt_info->sh_addr), (char*) section_offset,
		//				gotplt_info->sh_size);
	}

	//===================================================================
}

void load_from_elf_execve(task_struct_t *task, elf_sec_info_t* text_info,
		Elf64_Ehdr* temp, elf_sec_info_t* rodata_info,
		elf_sec_info_t* data_info, elf_sec_info_t* bss_info,
		elf_sec_info_t *ehframe_info, elf_sec_info_t *got_info,
		elf_sec_info_t *gotplt_info) {
	uint64_t section_offset;

	mem_desc_t * mem_desc_ptr = kmalloc(sizeof(struct mem_desc));
	mem_desc_ptr->num_vma = 0;
	mem_desc_ptr->vma_list = NULL;
	task->mem_map = mem_desc_ptr;
	if (text_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) text_info->sh_offset;
		mem_desc_ptr->text_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) text_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) text_info->sh_size;
		add_vma(vma_start, vma_end, 0, mem_desc_ptr);
		//		printf("text:  %x  %x  %x\n",text_info->sh_addr, section_offset, text_info->sh_size );
//		elf_mem_copy((char*) (text_info->sh_addr), (char*) section_offset,
//				(text_info->sh_size));
	}
	if (rodata_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) rodata_info->sh_offset;
		mem_desc_ptr->rodata_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) rodata_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) rodata_info->sh_size;
		add_vma(vma_start, vma_end, 1, mem_desc_ptr);

		//		printf("rodata:  %x  %x  %x\n",rodata_info->sh_addr, section_offset, rodata_info->sh_size );
//		elf_mem_copy((char*) (rodata_info->sh_addr), (char*) section_offset,
//				(rodata_info->sh_size));
	}
	if (data_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) data_info->sh_offset;
		mem_desc_ptr->data_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) data_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) data_info->sh_size;
		add_vma(vma_start, vma_end, 2, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",data_info->sh_addr, section_offset, data_info->sh_size );
		//		elf_mem_copy((char*) (data_info->sh_addr), (char*) section_offset,
		//				data_info->sh_size);
	}
	if (bss_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) bss_info->sh_offset;
		uint64_t vma_start = (uint64_t) bss_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) bss_info->sh_size;
		add_vma(vma_start, vma_end, 3, mem_desc_ptr);

		uint64_t heap_start = ((((uint64_t) vma_end) & (~(PAGE_SIZE - 1)))
				+ (PAGE_SIZE));
		mem_desc_ptr->brk = heap_start;
		add_vma(heap_start, heap_start, 5, mem_desc_ptr);
		//		printf("bss:  %x  %x  %x\n",bss_info->sh_addr, section_offset, bss_info->sh_size );
		//		elf_zerod_copy((char*) (bss_info->sh_addr), data_info->sh_size);
	}
	//================================================================
	if (ehframe_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) ehframe_info->sh_offset;
		mem_desc_ptr->ehframe_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) ehframe_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) ehframe_info->sh_size;
		add_vma(vma_start, vma_end, 6, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",ehframe_info->sh_addr, section_offset, ehframe_info->sh_size );
		//		elf_mem_copy((char*) (ehframe_info->sh_addr), (char*) section_offset,
		//				ehframe_info->sh_size);
	}
	if (got_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) got_info->sh_offset;
		mem_desc_ptr->got_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) got_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) got_info->sh_size;
		add_vma(vma_start, vma_end, 7, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",got_info->sh_addr, section_offset, got_info->sh_size );
		//		elf_mem_copy((char*) (got_info->sh_addr), (char*) section_offset,
		//				got_info->sh_size);
	}
	if (gotplt_info != NULL) {
		section_offset = (uint64_t) temp + (uint64_t) gotplt_info->sh_offset;
		mem_desc_ptr->gotplt_elf_addr = (char*) section_offset;
		uint64_t vma_start = (uint64_t) gotplt_info->sh_addr;
		uint64_t vma_end = vma_start + (uint64_t) gotplt_info->sh_size;
		add_vma(vma_start, vma_end, 8, mem_desc_ptr);

		//		printf("data:  %x  %x  %x\n",gotplt_info->sh_addr, section_offset, gotplt_info->sh_size );
		//		elf_mem_copy((char*) (gotplt_info->sh_addr), (char*) section_offset,
		//				gotplt_info->sh_size);
	}

	//===================================================================
}
uint64_t create_stack_vma(task_struct_t* currenttask) {
	uint64_t stack_page = 0x7000000;
	add_vma(stack_page, stack_page + 4096, 4, currenttask->mem_map); // stack vma mapping
	//todo: test stack demand paging by using apt user program
	currenttask->state.rsp = (uint64_t) stack_page + 0x500; //todo:change size to 1000
	// todo : hard-coding that process stack start is 0x7000000 + 0x500 in fork.c
	return stack_page;
}

//when processes are ready, I would like to make this a procedure where given the mem_desc I can load the executable into the memstruct
void load_executable(task_struct_t *currenttask) {
	char *str = currenttask->executable;
	struct posix_header_ustar *current =
			(struct posix_header_ustar *) &_binary_tarfs_start;
	int i = 0;
	elf_sec_info_t *text_info = NULL;
	elf_sec_info_t *rodata_info = NULL;
	elf_sec_info_t *data_info = NULL;
	elf_sec_info_t *bss_info = NULL;
	//===================================
	elf_sec_info_t *ehframe_info = NULL;
	elf_sec_info_t *got_info = NULL;
	elf_sec_info_t *gotplt_info = NULL;
	//====================================
	Elf64_Ehdr *temp = NULL;

	while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
		if (strcmp(current->name, str) == 0) {
//			printf("%s", current->name);
			uint64_t next = (uint64_t) ((uint64_t) current
					+ (uint64_t) sizeof(struct posix_header_ustar));
			temp = (Elf64_Ehdr *) (next);
			text_info = find_text_elf(temp);
			rodata_info = find_rodata_elf(temp);
			data_info = find_data_elf(temp);
			bss_info = find_bss_elf(temp);
			//================================
			ehframe_info = find_ehframe_elf(temp);
			got_info = find_got_elf(temp);
			gotplt_info = find_gotplt_elf(temp);
			//================================
			break;
		}
		//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
		uint64_t header_next = (uint64_t) ((align(
				convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))
				+ sizeof(struct posix_header_ustar) + (uint64_t) current);
//		printf("header : %x", header_next);
		current = (struct posix_header_ustar *) (header_next);
		i++;

	}
	load_from_elf(currenttask, text_info, temp, rodata_info, data_info,
			bss_info, ehframe_info, got_info, gotplt_info);

//	kfree(text_info);
//	kfree(rodata_info);
//	kfree(data_info);
//	kfree(bss_info);

	currenttask->state.rip = (uint64_t) (temp->e_entry);

	uint64_t stack_page = create_stack_vma(currenttask);
	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) stack_page, (uint64_t) free_frame);
	//add code to initialize stack
	//heap
}
inline int is_ELF(Elf64_Ehdr *temp) {
	if (temp->e_ident[0] == 0x7F && temp->e_ident[1] == 'E'
			&& temp->e_ident[2] == 'L' && temp->e_ident[3] == 'F')
		return 1;
	return 0;
}
int execve_error = 0;
executable_t *check_binary(char *binary) {
	struct posix_header_ustar *current =
			(struct posix_header_ustar *) &_binary_tarfs_start;
	int i = 0;

//		elf_sec_info_t *text_info = NULL;
//		elf_sec_info_t *rodata_info = NULL;
//		elf_sec_info_t *data_info = NULL;
//		elf_sec_info_t *bss_info = NULL;
	//===================================
//		elf_sec_info_t *ehframe_info = NULL;
//		elf_sec_info_t *got_info = NULL;
//		elf_sec_info_t *gotplt_info = NULL;
	//====================================
	Elf64_Ehdr *temp = NULL;

	while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
		if (strcmp(current->name, binary) == 0) {
			//			printf("%s", current->name);
//			char amb = '5';
//			if(current->typeflag == &amb){
//				return NULL;
//			}
			uint64_t next = (uint64_t) ((uint64_t) current
					+ (uint64_t) sizeof(struct posix_header_ustar));

			temp = (Elf64_Ehdr *) (next);
			executable_t *temp1 = kmalloc(sizeof(executable_t));
			temp1->bss_info = NULL;
			temp1->data_info = NULL;
			temp1->ehframe_info = NULL;
			temp1->got_info = NULL;
			temp1->gotplt_info = NULL;
			temp1->rodata_info = NULL;
			temp1->text_info = NULL;
			temp1->is_script = 0;
			temp1->temp = temp;
			if (is_ELF(temp) == 1) {
				temp1->text_info = find_text_elf(temp);
				temp1->rodata_info = find_rodata_elf(temp);
				temp1->data_info = find_data_elf(temp);
				temp1->bss_info = find_bss_elf(temp);
				//================================
				temp1->ehframe_info = find_ehframe_elf(temp);
				temp1->got_info = find_got_elf(temp);
				temp1->gotplt_info = find_gotplt_elf(temp);
				temp1->temp = temp;
				//================================
				temp1->is_script = 0;
				return temp1;
			} else {
				temp1->is_script = 1;
				return temp1;
			}
		}
		//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
		uint64_t header_next = (uint64_t) ((align(
				convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))
				+ sizeof(struct posix_header_ustar) + (uint64_t) current);
		//		printf("header : %x", header_next);
		current = (struct posix_header_ustar *) (header_next);
		i++;

	}
	return NULL;
}
/*
 *
 */
/*
 * PROCESS Global variables section
 *
 */
struct task_struct *init_task = NULL;
static uint64_t pid_counter = 2;
/*
 * end of PROCESS Global Variables
 */
uint64_t get_next_pid() {
	uint64_t pid_value = pid_counter;
	pid_counter += 1;//assumption we never go above max value of uint64_t reasonable assumption
	return pid_value;
}
void init_text_vm() {

}
void init_vm(struct mem_desc *mem_map) {
	vma_t * vma_ptr = kmalloc(sizeof(struct vma));
	// head vma of kernel
	vma_ptr->my_mem_desc = mem_map;
	vma_ptr->vma_start = 0;
	vma_ptr->vma_end = 0;
	mem_map->vma_list = vma_ptr;
}
void map_process_vm(task_struct_t *task) {
	task->mem_map = kmalloc(sizeof(struct mem_desc));
	init_vm(task->mem_map);
}
//todo:initialize process state function init_process_state

/*
 * rememver:
 * Kernel space is flagged in the page tables as exclusive to privileged code (ring 2 or lower), hence a page fault is triggered
 * if user-mode programs try to touch it
 */

void maintasktwo() {
	printf("\nthis is main task two ");
	while (1)
		;
//	preempt();
}
void stack_ring_three(task_struct_t *task) {
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);
	__asm__ __volatile__("movw $0x2B,%%ax\n\t"
			"ltr %%ax"
			:::"rax"
	);

}

inline void quit_kernel_thread() {
	int n = 60;
	int a1 = 0;
	int result;
	__asm__ __volatile(
			"int $0x80"
			:"=&a" (result)
			:"0"(n),"D"(a1));
}
void test_main() {
	uint64_t i = 1;
	printf("inside kernel thread %d", i);
	limit = limit << 2;
	while (i < limit) {
		//if(i%100000000 == 0){
		//	printf("%d ", i);
		//	i++;
		//}
		i++;
	}
	quit_kernel_thread();
}

void init_global_fd() {
	//creating a secon process and that process has the same globals as the init process
	stdin_fd = (file_desc_t *) kmalloc(sizeof(file_desc_t));
	stdout_fd = (file_desc_t *) kmalloc(sizeof(file_desc_t));
	input_buffer = (void *) kmalloc(0x1000);
	stdin_fd->posix_header = NULL;
	stdin_fd->current_pointer = (char *) input_buffer;
	global_keyboard = input_buffer;
	stdin_fd->flags = O_RDONLY;
	stdin_fd->size = 0x1000;
	stdin_fd->used = 1;
	stdin_fd->busy = 0;
	stdin_fd->current_process = 0;
	stdin_fd->ready = 0;
	stdin_fd->posix_header = NULL;
	stdout_fd->current_pointer = (char *) BASE_CURSOR_POS;
	stdout_fd->flags = O_WRONLY;
	stdout_fd->size = 0x1000;
	stdout_fd->used = 1;
	stdout_fd->busy = 0;
	stdout_fd->current_process = 0;
	stdout_fd->ready = 0;
	current_stdin_pointer = stdin_fd->current_pointer;
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		global_fd_array[i].fd = NULL;
		global_fd_array[i].count = 0;
	}
	global_fd_array[0].fd = stdin_fd;
	global_fd_array[0].count = 1;
	global_fd_array[1].fd = stdout_fd;
	global_fd_array[1].count = 1;
}

int increment_global_count_fd(file_desc_t *fd) {
	int check = 0;
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		if (global_fd_array[i].fd != NULL && global_fd_array[i].fd == fd) {
			check = 1;
			(global_fd_array[i].count)++;
			break;
		}
	}
	if (check == 0) {
		return add_to_global_fd(fd);
	}
	return 1;
}
void decrement_global_count_fd(file_desc_t *fd) {
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		if (global_fd_array[i].fd != NULL && global_fd_array[i].fd == fd) {
			(global_fd_array[i].count)--;
		}
		//todo: add code to delete an fd if the count drops to 0
	}
}
int add_to_global_fd(file_desc_t *fd) {
	for (uint64_t i = 0; i < MAX_FILES_OS; i++) {
		if (global_fd_array[i].fd == NULL) {
			global_fd_array[i].fd = fd;
			global_fd_array[i].count = 1;
			return 1;
		}
	}
	return 0;
}
void add_default_env(task_struct_t *task) {
	uint64_t oldcr3 = 0;
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(oldcr3)
			:
			:"%rax");
	update_cr3((uint64_t *) (task->state.cr3));

	add_vma(ENV_START, ENV_END, 20, task->mem_map);
	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) ENV_START, (uint64_t) free_frame);
	char *current_env = (char *) ENV_START;
	char *env1 = current_env;
	copy_string("PATH=bin/", &current_env);
	char *argv1 = current_env;
	copy_string(task->executable, &current_env);
	uint64_t *stack_top = (uint64_t *) (task->state.rsp);
	*stack_top = 0;
	stack_top--;
	*stack_top = (uint64_t) env1;
	stack_top--;
	*stack_top = 0;
	stack_top--;
	*stack_top = (uint64_t) argv1;
	stack_top--;
	*stack_top = 1;
//	stack_top--;
	regs_syscall_t *regs = (regs_syscall_t *) (task->state.kernel_rsp);
	uint64_t *user_stack_return = (uint64_t *) (&(regs->rsp));
	*user_stack_return = (uint64_t) stack_top;
	update_cr3((uint64_t *) (oldcr3));
}
void kernel_process_init() {
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
	init_global_fd();
	currenttask = &taskone;
	currenttask->executable[0] = '\0';
	currenttask->pwd[0] = '\0';
	currenttask->next = currenttask;
	currenttask->pid = 1;
	currenttask->is_kernel_process = 1;
	stack_ring_three(currenttask);
	lasttask = currenttask;
//	tss.rsp0 = (uint64_t) (currenttask->state.kernel_rsp);
	init_file_dp_process(currenttask);
//	temp_create_kernel_process(test_main,1);
	temp_create_kernel_process(waiting_to_running_q, 1);
	temp_create_kernel_process(check_user_process_waitpid_daemon, 1);
	temp_create_kernel_process(return_blocking_rw_to_runq, 1);
	temp_create_kernel_process(clean_up_processes, 1);
	ENV_SWAP_START = kmalloc(0x1000);
	STACK_SWAP_START = kmalloc(0x1000);
	temp_create_user_process("bin/sbush", 1);
	task_struct_t *temp = currenttask;
	do {
		if (temp->is_kernel_process == 0) {
			add_default_env(temp);
		}
		temp = temp->next;
	} while (temp != currenttask);

//	temp_create_kernel_process(clear_keyboard_busy, 1);
//	temp_create_user_process("bin/hello", 4);
//	temp_create_user_process("bin/hello2", 1);
//
//	temp_create_user_process("bin/hello", 4);
//	temp_create_kernel_process(test_main,1);
//	temp_create_kernel_process(test_main,1);
//	temp_create_kernel_process(test_main,1);
//	temp_create_user_process("bin/hello", 1);
	__asm__ __volatile__("sti");
	while (1) {
		__asm__ __volatile("sti");
	}

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

void add_kernel_stack(task_struct_t* task) {
	//giving the process a new kernel stack
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);
}

//named kernel_create_process but actually creates user process, we need to change all this to name it properly once we both have a stable merge

uint64_t temp_preempt(uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
	if (currenttask == &taskone) {
		currenttask = currenttask->next;
	}
	if (currenttask == last) {
		return stack_top;
	}
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
//	if(currenttask->pid == 2){
//		printf("tss rsp %x", tss.rsp0);
//	}
//	__asm__ __volatile__("movq %0, %%rsp"
//						:
//						:"r"(currenttask->state.kernel_rsp)
//						:"%rsp");
//	printf("%p", tss.rsp0);
	update_cr3((uint64_t *) (currenttask->state.cr3));
//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);

	//	printf("%p ", tss.rsp0);

}

uint64_t temp_preempt_read_block(uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
//		if(currenttask == &taskone){
//			currenttask = currenttask->next;
//		}
//		if(currenttask == last){
//			return stack_top;
//		}
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
	//	__asm__ __volatile__("movq %0, %%rsp"
	//						:
	//						:"r"(currenttask->state.kernel_rsp)
	//						:"%rsp");
	//	printf("%p", tss.rsp0);
	last->p_state = STATE_WAITING;
	move_process_runq_to_waitq(last->pid);
	update_cr3((uint64_t *) (currenttask->state.cr3));
	//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);
}
uint64_t temp_preempt_wait(int fd, void *buffer, uint64_t size,
		uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
//		if(currenttask == &taskone){
//			currenttask = currenttask->next;
//		}
//		if(currenttask == last){
//			return stack_top;
//		}
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
	//	__asm__ __volatile__("movq %0, %%rsp"
	//						:
	//						:"r"(currenttask->state.kernel_rsp)
	//						:"%rsp");
	//	printf("%p", tss.rsp0);
	last->p_state = STATE_WAITING;

	temp_create_kernel_process_read(read_thread_process, last->pid, fd, buffer,
			size);	//this is actually read, hard to refactor, a mess it is
	move_process_runq_to_waitq(last->pid);
	update_cr3((uint64_t *) (currenttask->state.cr3));
	//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);
}

uint64_t temp_preempt_write(int fd, void *buffer, uint64_t size,
		uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
	last->p_state = STATE_WAITING;
	temp_create_kernel_process_write(write_thread_process, last->pid, fd,
			buffer, size);
	move_process_runq_to_waitq(last->pid);
	update_cr3((uint64_t *) (currenttask->state.cr3));
	//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);
}
uint64_t temp_preempt_waitpid(int pid, int *status, int options,
		uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
//		if(currenttask == &taskone){
//			currenttask = currenttask->next;
//		}
//		if(currenttask == last){
//			return stack_top;
//		}
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
	//	__asm__ __volatile__("movq %0, %%rsp"
	//						:
	//						:"r"(currenttask->state.kernel_rsp)
	//						:"%rsp");
	//	printf("%p", tss.rsp0);
	last->p_state = STATE_WAITING;
	last->waiting_for = pid;
	move_process_runq_to_waitq(last->pid);
	update_cr3((uint64_t *) (currenttask->state.cr3));
	//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);
}

uint64_t temp_preempt_nanosleep(const struct timespec *rqtp,
		struct timespec *rmtp, uint64_t stack_top) {
	if (currenttask == &taskone && currenttask->next == currenttask) {
		return stack_top;
	}
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
//		if(currenttask == &taskone){
//			currenttask = currenttask->next;
//		}
//		if(currenttask == last){
//			return stack_top;
//		}
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(last->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	//	printf("h\n");
	if (currenttask->is_kernel_process != 1)
		tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
	//	__asm__ __volatile__("movq %0, %%rsp"
	//						:
	//						:"r"(currenttask->state.kernel_rsp)
	//						:"%rsp");
	//	printf("%p", tss.rsp0);
	last->p_state = STATE_WAITING;
	move_process_runq_to_waitq(last->pid);
	nanosleep_node_t *temp = make_nanosleep_node(rqtp, last);
	add_nanosleep_list(temp);
	update_cr3((uint64_t *) (currenttask->state.cr3));
	//	printf("pid is %d ",currenttask->pid);
	return (currenttask->state.kernel_rsp);
}

void make_new_process_state(task_struct_t *task, task_struct_t *parent_task,
		executable_t *executable) {
	task->mem_map = NULL;
	task->ppid = parent_task->pid;
	printf(" task pid : %d task ppid %d\n", task->pid, task->ppid);
	strcpy(task->pwd, parent_task->pwd);
	task->is_background = 0;
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 0;
	task->state.cr3 = parent_task->state.cr3;
	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	uint64_t *temp1 = get_physical_pml4_base_for_process();
	uint64_t oldcr3 = 0;
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(oldcr3)
			:
			:"%rax");
	update_cr3((uint64_t *) temp1);
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(task->state.cr3)
			:
			:"%rax");
	map_process_vm(task);
	load_from_elf_execve(task, executable->text_info, executable->temp,
			executable->rodata_info, executable->data_info,
			executable->bss_info, executable->ehframe_info,
			executable->got_info, executable->gotplt_info);

	task->state.rip = (uint64_t) (executable->temp->e_entry);
	uint64_t stack_page = create_stack_vma(currenttask);
	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) stack_page, (uint64_t) free_frame);

	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);
	temp_init_user_stack(task->state.kernel_rsp, task);
	task->p_state = STATE_RUNNING;
//	copy_file_dp_process(task, parent_task);
	currenttask = task;
//	update_cr3((uint64_t *)(oldcr3));
}

void make_new_process_state_background(task_struct_t *task,
		task_struct_t *parent_task, executable_t *executable) {
	task->mem_map = NULL;
	task->ppid = parent_task->pid;
	task->is_background = 1;
	strcpy(task->pwd, parent_task->pwd);
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 0;
	task->state.cr3 = parent_task->state.cr3;
	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	uint64_t *temp1 = get_physical_pml4_base_for_process();
	uint64_t oldcr3 = 0;
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(oldcr3)
			:
			:"%rax");
	update_cr3((uint64_t *) temp1);
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(task->state.cr3)
			:
			:"%rax");
	map_process_vm(task);
	load_from_elf_execve(task, executable->text_info, executable->temp,
			executable->rodata_info, executable->data_info,
			executable->bss_info, executable->ehframe_info,
			executable->got_info, executable->gotplt_info);

	task->state.rip = (uint64_t) (executable->temp->e_entry);
	uint64_t stack_page = create_stack_vma(currenttask);
	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) stack_page, (uint64_t) free_frame);

	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);
	temp_init_user_stack(task->state.kernel_rsp, task);
	task->p_state = STATE_RUNNING;
//	copy_file_dp_process(task, parent_task);
	currenttask = task;
//	update_cr3((uint64_t *)(oldcr3));
}
task_struct_t* get_parent_from_ppid() {
	task_struct_t* temp_start = currenttask->next;
	task_struct_t* parent_task = NULL;
	while (temp_start->pid != currenttask->ppid && temp_start != currenttask) {
		temp_start = temp_start->next;
	}
	if (temp_start->pid == currenttask->ppid)
		parent_task = temp_start;
	else {
		temp_start = waitingtask->next;
		parent_task = NULL;
		while (temp_start->pid != currenttask->ppid && temp_start != waitingtask) {
			temp_start = temp_start->next;
		}
		if (temp_start->pid == currenttask->ppid) {
			parent_task = temp_start;
		} else {
			temp_start = currenttask->next;
			while (temp_start->pid != 1) {
				temp_start = temp_start->next;
			}
			parent_task = temp_start;
		}
	}
	return parent_task;
}

void make_new_process(executable_t *executable) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	copy_file_dp_process(task, currenttask);
	int pid = currenttask->pid;
	task->pid = pid;
	task->ppid = currenttask->ppid;
	printf(" 1: task pid : %d task ppid: %d \n", task->pid, task->ppid);
	task_struct_t* parent_task = get_parent_from_ppid();
	if (currenttask->next == currenttask) {
		currenttask->next = task;
		task->next = currenttask;
//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask=currenttask->next;
		currenttask->pid = -1;
		currenttask->p_state = STATE_TERMINATED;
		move_process_runq_to_waitq(-1);
		currenttask = task;
	}
	make_new_process_state(task, parent_task, executable);

}

void make_new_process_background(executable_t *executable) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	copy_file_dp_process_background(task, currenttask);
	int pid = currenttask->pid;
	task->pid = pid;
	task->ppid = currenttask->ppid;
	task_struct_t* parent_task = get_parent_from_ppid();
	if (currenttask->next == currenttask) {
		currenttask->next = task;
		task->next = currenttask;
//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask=currenttask->next;
		currenttask->pid = -1;
		currenttask->p_state = STATE_TERMINATED;
		move_process_runq_to_waitq(-1);
		currenttask = task;
	}
	make_new_process_state_background(task, parent_task, executable);

}
void add_env_to_stack(task_struct_t *task) {
	add_vma(ENV_START, ENV_END, 20, task->mem_map);
	char *current_env = (char *) ENV_START;
	uint64_t *temp_stack = (uint64_t *) (STACK_SWAP_START);
	uint64_t *stack_top = (uint64_t *) (task->state.rsp);
	int num_nulls = 0;
	while (num_nulls < 2) {
		if (*temp_stack == 0) {
			num_nulls++;
		}
		temp_stack++;
	}
	temp_stack--;
	while (temp_stack != (uint64_t *) STACK_SWAP_START) {
		if (*temp_stack == 0) {
			*stack_top = 0;
		} else {
			*stack_top = (uint64_t) current_env;
			copy_string((char *) *temp_stack, &current_env);
		}
		temp_stack--;
		stack_top--;
	}
	*stack_top = *temp_stack;
	regs_syscall_t *regs = (regs_syscall_t *) (task->state.kernel_rsp);
	uint64_t *user_stack_return = (uint64_t *) (&(regs->rsp));
	*user_stack_return = (uint64_t) stack_top;
}
uint64_t setup_new_process(char *binary, char *argv[], char *envp[],
		executable_t *executable) {

	uint64_t *environ_temp_start = (uint64_t *) (ENV_SWAP_START);
	uint64_t *temp_stack = (uint64_t *) (STACK_SWAP_START);

	char *current_environ = (char *) (environ_temp_start);
	uint64_t *current_stack = (uint64_t *) (temp_stack);
	uint64_t *argv_count = temp_stack;
	int i = 0;
	char *current = (char *) argv[i];
	*argv_count = 0;
	current_stack++;
	//copying the arg variables
	while (current != NULL) {
		(*argv_count)++;
		*current_stack = (uint64_t) (current_environ);
		current_stack++;
		copy_string((char *) (current), &current_environ);
		i++;
		current = argv[i];
	}
	*current_stack = 0;
	current_stack++;
	i = 0;
	current = (char *) envp[i];
	//copying the environment variables.
	while (current != NULL) {
		*current_stack = (uint64_t) (current_environ);
		current_stack++;
		copy_string((char *) (current), &current_environ);
		i++;
		current = envp[i];

	}
	*current_stack = 0;
	make_new_process(executable);
//	load_from_elf_execve(currenttask, executable->text_info, executable->temp, executable->rodata_info, executable->data_info,
//			executable->bss_info, executable->ehframe_info, executable->got_info, execuatable->gotplt_info);

	//	kfree(text_info);
	//	kfree(rodata_info);
	//	kfree(data_info);
	//	kfree(bss_info);

	currenttask->state.rip = (uint64_t) (executable->temp->e_entry);
	uint64_t stack_page = create_stack_vma(currenttask);

	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) stack_page, (uint64_t) free_frame);
	add_env_to_stack(currenttask);
	return currenttask->state.kernel_rsp;

}

uint64_t setup_new_background_process(char *binary, char *argv[], char *envp[],
		executable_t *executable) {

	uint64_t *environ_temp_start = (uint64_t *) (ENV_SWAP_START);
	uint64_t *temp_stack = (uint64_t *) (STACK_SWAP_START);

	char *current_environ = (char *) (environ_temp_start);
	uint64_t *current_stack = (uint64_t *) (temp_stack);
	uint64_t *argv_count = temp_stack;
	int i = 0;
	char *current = (char *) argv[i];
	*argv_count = 0;
	current_stack++;
	//copying the arg variables
	while (current != NULL) {
		(*argv_count)++;
		*current_stack = (uint64_t) (current_environ);
		current_stack++;
		copy_string((char *) (current), &current_environ);
		i++;
		current = argv[i];
	}
	*current_stack = 0;
	current_stack++;
	i = 0;
	current = (char *) envp[i];
	//copying the environment variables.
	while (current != NULL) {
		*current_stack = (uint64_t) (current_environ);
		current_stack++;
		copy_string((char *) (current), &current_environ);
		i++;
		current = envp[i];

	}
	*current_stack = 0;
	make_new_process_background(executable);
//	load_from_elf_execve(currenttask, executable->text_info, executable->temp, executable->rodata_info, executable->data_info,
//			executable->bss_info, executable->ehframe_info, executable->got_info, execuatable->gotplt_info);

	//	kfree(text_info);
	//	kfree(rodata_info);
	//	kfree(data_info);
	//	kfree(bss_info);

	currenttask->state.rip = (uint64_t) (executable->temp->e_entry);
	uint64_t stack_page = create_stack_vma(currenttask);

	void *free_frame = (void *) get_free_frames(0);
	setup_process_page_tables((uint64_t) stack_page, (uint64_t) free_frame);
	add_env_to_stack(currenttask);
	return currenttask->state.kernel_rsp;

}

uint64_t execve_process(char *binary, char **argv, char **envp,
		uint64_t stack_top) {
	__asm__ __volatile__("movq %1, %%rax\n\t"
			"movq %%rax, %0"
			:"=r"(currenttask->state.kernel_rsp)
			:"r"(stack_top)
			:"memory", "%rax", "%rsp");
	executable_t *executable = check_binary(binary);
	regs_syscall_t *regs = (regs_syscall_t *) (currenttask->state.kernel_rsp);

	if (executable == NULL) {
		regs->rax = (uint64_t) -EACCES;
		return (uint64_t) regs;
	}
	char **argv_temp = argv;
	int i = 0;
	while (argv_temp[i] != NULL) {

		i++;
	}
	char amb = '&';
	if (*(argv_temp[i - 1]) == amb) {
		argv_temp[i - 1] = NULL;
		return setup_new_background_process(binary, (char **) argv,
				(char **) envp, executable);
	}
	return setup_new_process(binary, (char **) argv, (char **) envp, executable);

}
void check_parent_waiting(task_struct_t *last) {
	int ppid = last->ppid;
	task_struct_t *temp = waitingtask;
	if (temp == NULL)
		return;
	if (last->is_kernel_process == 1)
		return;
	if (last->is_background == 1)
		return;
	do {
		if (temp->pid == ppid
				&& (temp->waiting_for == last->pid || temp->waiting_for == -1)) {
			temp->waiting_for = DEFAULT_WAITING_FOR;
			regs_syscall_t *regs = (regs_syscall_t *) temp->state.kernel_rsp;
			regs->rax = last->pid;
			temp->p_state = STATE_READY;
//			move_process_waitq_to_runq(ppid);
			break;
		}
		temp = temp->next;
	} while (temp != waitingtask);
}

void mark_as_terminated(task_struct_t* last) {
	task_struct_t *prev = currenttask;
	while (prev->next != last) {
		prev = prev->next;
	}
	prev->next = last->next;

	last->next = last;
	last->p_state = STATE_TERMINATED;
	last->pid = -1;
	add_process_waitq(last);
	check_parent_waiting(last);
}

void space_msg() {
//	printf(" #dirty free-pages %d \n", num_free_pages(1));
//	printf(" #zeroed free-pages %d \n", num_free_pages(0));
//	printf(" #tot free-pages %d \n", num_free_pages(2));
}

uint64_t temp_preempt_exit(uint64_t stack_top) {
	//printf("inside preempt exit %d iskernel:%d", currenttask->pid, currenttask->is_kernel_process);
	task_struct_t *last = currenttask;
	currenttask = currenttask->next;
//	process_switch_cooperative(&(last->state),&(currenttask->state), stack_top);
//	tss.rsp0 = (uint64_t) (currenttask->state.kernel_rsp);
	mark_as_terminated(last);

	tss.rsp0 = (uint64_t) ((currenttask->state.kernel_rsp) + 192);
//	__asm__ __volatile__("movq %0, %%rsp"
//							:
//							:"r"(currenttask->state.kernel_rsp)
//							:"%rsp");
	update_cr3((uint64_t *) (currenttask->state.cr3));

	space_msg();
//	cleanup_process(last);
//	printf(" num freepages after %d ", num_free_pages());

	return (currenttask->state.kernel_rsp);
}

void temp_create_user_process(char *executable, uint64_t ppid) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	task_struct_t *temp_start = currenttask->next;
	task_struct_t *parent_task = NULL;
	while (temp_start->pid != ppid) {
		temp_start = temp_start->next;
	}
	//assumption here is that the process has a parent, then the parent is found, have to handle zombie process somehow need to know how.
	parent_task = temp_start;
	//add the process to the end of list
	if (parent_task->next == parent_task) {
		parent_task->next = task;
		task->next = parent_task;

//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask = task;
//		lasttask = currenttask;
	}
	temp_init_user_state(task, parent_task, executable);

}
void init_file_dp_process(task_struct_t* task) {
	for (int i = 0; i < 50; i++) {
		task->filearray[i] = NULL;
	}
	task->filearray[0] = stdin_fd;
	task->filearray[1] = stdout_fd;
	task->filearray[2] = stdout_fd;
	for (int i = 0; i < 50; i++) {
		if (task->filearray[i] != NULL) {
			increment_global_count_fd(task->filearray[i]);
		}
	}
}

void temp_init_user_state(task_struct_t *task, task_struct_t *parent_task,
		char *executable) {
	//using the user level process struct it is fine for now
	task->mem_map = NULL;
	task->pid = get_next_pid();
	task->ppid = parent_task->pid;	//this need to be more involved.
	task->is_background = 0;
	strcpy(task->pwd, parent_task->pwd);
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 0;
	//	task->state.rip = (uint64_t) main;
	task->state.cr3 = parent_task->state.cr3;
	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	// need to assign a new stack and since it grows down, we need to change taht to the end of the page too.
	strcpy((*task).executable, executable);
	uint64_t *temp = get_physical_pml4_base_for_process();
	////
	uint64_t oldcr3 = 0;
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(oldcr3)
			:
			:"%rax");
	update_cr3((uint64_t *) (temp));
	__asm__ __volatile__("movq %%cr3, %%rax\n\t"
			"movq %%rax, %0\n\t"
			:"=m"(task->state.cr3)
			:
			:"%rax");
	//	currenttask->state.cr3 = (uint64_t)temp;
	map_process_vm(task);
	if (task->executable[0] != '\0')
		load_executable(task);
	update_cr3((uint64_t *) (oldcr3));
	//giving the process a new kernel stack
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);

	temp_init_user_stack(task->state.kernel_rsp, task);
	//adding fds
	task->p_state = STATE_RUNNING;
	copy_file_dp_process(task, parent_task);
}

void temp_init_user_stack(uint64_t rsp, task_struct_t *task) {
	uint64_t *temp = (uint64_t *) rsp;
//	printf("--task: %p\n", rsp);
	*temp = USER_DATA;	//ss
	temp -= 1;
	*temp = task->state.rsp; //rsp
	temp -= 1;
	*temp = task->state.flags; //flags
	temp -= 1;
	*temp = USER_CODE; //cs
	temp -= 1;
	*temp = task->state.rip; //ip
	temp -= 1;
	*temp = 0; //rax
	temp -= 1;
	*temp = 0; //rbx
	temp -= 1;
	*temp = 0; //rcx
	temp -= 1;
	*temp = 0; //rdx
	temp -= 1;
	*temp = 0; //rdi
	temp -= 1;
	*temp = 0; //rsi
	temp -= 1;
	*temp = 0; //rbp
	temp -= 1;
	*temp = 0; //r8
	temp -= 1;
	*temp = 0; //r9
	temp -= 1;
	*temp = 0; //r10
	temp -= 1;
	*temp = 0; //r11
	temp -= 1;
	*temp = 0; //r12
	temp -= 1;
	*temp = 0; //r13
	temp -= 1;
	*temp = 0; //r14
	temp -= 1;
	*temp = 0; //r15
	temp -= 1;
	*temp = USER_DATA; //ds
	temp -= 1;
	*temp = USER_DATA; //es
	temp -= 1;
	*temp = USER_DATA; //fs
	temp -= 1;
	*temp = USER_DATA; //gs
	task->state.kernel_rsp = (uint64_t) temp;
//	printf("task: %p\n", task->state.kernel_rsp);
}

void temp_create_kernel_process(void (*main)(), uint64_t ppid) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	task_struct_t *temp_start = currenttask->next;
	task_struct_t *parent_task = NULL;

	while (temp_start->pid != ppid) {
		temp_start = temp_start->next;
	}
	//assumption here is that the process has a parent, then the parent is found, have to handle zombie process somehow need to know how.
	parent_task = temp_start;
	//add the process to the end of list
	if (parent_task->next == parent_task) {
		parent_task->next = task;
		task->next = parent_task;

//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask = task;
//		lasttask = currenttask;
	}
	temp_init_kernel_state(task, parent_task, main);

}

void temp_init_kernel_state(task_struct_t *task, task_struct_t *parent_task,
		void (*main)()) {
	task->mem_map = NULL;
	task->pid = get_next_pid();
	task->ppid = parent_task->pid; //this need to be more involved.
	task->is_background = 0;
	strcpy(task->pwd, parent_task->pwd);
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 1;
	task->ppid = parent_task->pid; //this need to be more involved.
	task->state.rip = (uint64_t) main;
	task->state.cr3 = parent_task->state.cr3;

	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	// need to assign a new stack and since it grows down, we need to change taht to the end of the page too.
	//giving the process a new kernel stack
	uint64_t stack_kernel_process = (uint64_t) kmalloc(0x1000);
	task->state.rsp = (uint64_t) (stack_kernel_process + 0xfff);
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);

	temp_init_kernel_stack(task->state.kernel_rsp, task);
	copy_file_dp_process(task, parent_task);
	task->p_state = STATE_RUNNING;
}

void temp_init_kernel_stack(uint64_t rsp, task_struct_t *task) {
	uint64_t *temp = (uint64_t *) rsp;
	*temp = KERNEL_DATA;	//ss
	temp -= 1;
	*temp = task->state.rsp; //rsp
	temp -= 1;
	*temp = task->state.flags; //flags
	temp -= 1;
	*temp = KERNEL_CODE; //cs
	temp -= 1;
	*temp = task->state.rip; //ip
	temp -= 1;
	*temp = 0; //rax
	temp -= 1;
	*temp = 0; //rbx
	temp -= 1;
	*temp = 0; //rcx
	temp -= 1;
	*temp = 0; //rdx
	temp -= 1;
	*temp = 0; //rdi
	temp -= 1;
	*temp = 0; //rsi
	temp -= 1;
	*temp = 0; //rbp
	temp -= 1;
	*temp = 0; //r8
	temp -= 1;
	*temp = 0; //r9
	temp -= 1;
	*temp = 0; //r10
	temp -= 1;
	*temp = 0; //r11
	temp -= 1;
	*temp = 0; //r12
	temp -= 1;
	*temp = 0; //r13
	temp -= 1;
	*temp = 0; //r14
	temp -= 1;
	*temp = 0; //r15
	temp -= 1;
	*temp = KERNEL_DATA; //ds
	temp -= 1;
	*temp = KERNEL_DATA; //es
	temp -= 1;
	*temp = KERNEL_DATA; //fs
	temp -= 1;
	*temp = KERNEL_DATA; //gs
	task->state.kernel_rsp = (uint64_t) temp;
//	printf("task: %p\n", task->state.kernel_rsp);
}

//========================================================
//this is actually read, hard to refactor, a mess it is
void temp_create_kernel_process_read(void (*main)(), uint64_t ppid, int fd,
		void *buffer, uint64_t size) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	task_struct_t *temp_start = currenttask->next;
	task_struct_t *parent_task = NULL;
	while (temp_start->pid != ppid) {
		temp_start = temp_start->next;
	}
	//assumption here is that the process has a parent, then the parent is found, have to handle zombie process somehow need to know how.
	parent_task = temp_start;
	//add the process to the end of list
	if (parent_task->next == parent_task) {
		parent_task->next = task;
		task->next = parent_task;

//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask = task;
//		lasttask = currenttask;
	}
	temp_init_kernel_state_read(task, parent_task, main, fd, buffer, size);

}

void temp_create_kernel_process_write(void (*main)(), uint64_t ppid, int fd,
		void *buffer, uint64_t size) {
	task_struct_t *task = kmalloc(sizeof(task_struct_t));
	task_struct_t *temp_start = currenttask->next;
	task_struct_t *parent_task = NULL;
	while (temp_start->pid != ppid) {
		temp_start = temp_start->next;
	}
	//assumption here is that the process has a parent, then the parent is found, have to handle zombie process somehow need to know how.
	parent_task = temp_start;
	//add the process to the end of list
	if (parent_task->next == parent_task) {
		parent_task->next = task;
		task->next = parent_task;

//		lasttask = task;
	} else {
		task->next = lasttask->next;
		lasttask->next = task;
//		lasttask = task;
//		lasttask = ;
	}
	temp_init_kernel_state_write(task, parent_task, main, fd, buffer, size);

}
void copy_file_dp_process(task_struct_t *task, task_struct_t *parent_task) {
	for (uint64_t i = 0; i < MAX_NUMBER_FILES; i++) {
		task->filearray[i] = parent_task->filearray[i];
		if (task->filearray[i] != NULL) {
			increment_global_count_fd(task->filearray[i]);
		}
	}
}
void copy_file_dp_process_background(task_struct_t *task,
		task_struct_t *parent_task) {
	for (uint64_t i = 0; i < MAX_NUMBER_FILES; i++) {
		if (parent_task->filearray[i] != stdin_fd
				&& parent_task->filearray[i] != stdout_fd) {
			task->filearray[i] = parent_task->filearray[i];
			if (task->filearray[i] != NULL) {
				increment_global_count_fd(task->filearray[i]);
			}
		} else {
			task->filearray[i] = NULL;
		}
	}
}
//this is actually read, hard to refactor, a mess it is
void temp_init_kernel_state_read(task_struct_t *task,
		task_struct_t *parent_task, void (*main)(), int fd, void *buffer,
		uint64_t size) {
	task->mem_map = parent_task->mem_map;
	task->pid = get_next_pid();
	task->ppid = parent_task->pid; //this need to be more involved.
	task->is_background = 0;
	;
	task->state.rip = (uint64_t) main;
	strcpy(task->pwd, parent_task->pwd);
	task->state.cr3 = parent_task->state.cr3;
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 1;
	strcpy(task->executable, parent_task->executable);
	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	// need to assign a new stack and since it grows down, we need to change taht to the end of the page too.
	//giving the process a new kernel stack
	uint64_t stack_kernel_process = (uint64_t) kmalloc(0x1000);
	task->state.rsp = (uint64_t) (stack_kernel_process + 0xfff);
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);

	temp_init_kernel_stack_read(task->state.kernel_rsp, task, fd, buffer, size);
//	init_file_dp_process(task);
	copy_file_dp_process(task, parent_task);
	task->p_state = STATE_RUNNING;
}

void temp_init_kernel_state_write(task_struct_t *task,
		task_struct_t *parent_task, void (*main)(), int fd, void *buffer,
		uint64_t size) {
	task->mem_map = parent_task->mem_map;
	task->pid = get_next_pid();
	task->ppid = parent_task->pid; //this need to be more involved.
	task->is_background = 0;
	strcpy(task->pwd, parent_task->pwd);
	task->state.rip = (uint64_t) main;
	task->state.cr3 = parent_task->state.cr3;
	task->waiting_for = DEFAULT_WAITING_FOR;
	task->is_kernel_process = 1;
	strcpy(task->executable, parent_task->executable);
	task->state.flags = parent_task->state.flags;
	task->state.flags |= 0x200;
	// need to assign a new stack and since it grows down, we need to change taht to the end of the page too.
	//giving the process a new kernel stack
	uint64_t stack_kernel_process = (uint64_t) kmalloc(0x1000);
	task->state.rsp = (uint64_t) (stack_kernel_process + 0xfff);
	uint64_t stack_kernel = (uint64_t) kmalloc(0x1000);
	task->state.kernel_rsp = (uint64_t) (stack_kernel + 0xfff);

	temp_init_kernel_stack_write(task->state.kernel_rsp, task, fd, buffer,
			size);
//	init_file_dp_process(task);
	copy_file_dp_process(task, parent_task);
	task->p_state = STATE_RUNNING;
}
//this is actually read, hard to refactor, a mess it is
void temp_init_kernel_stack_read(uint64_t rsp, task_struct_t *task, int fd,
		void *buffer, uint64_t size) {
	uint64_t *temp = (uint64_t *) rsp;
	*temp = KERNEL_DATA;	//ss
	temp -= 1;
	*temp = task->state.rsp; //rsp
	temp -= 1;
	*temp = task->state.flags; //flags
	temp -= 1;
	*temp = KERNEL_CODE; //cs
	temp -= 1;
	*temp = task->state.rip; //ip
	temp -= 1;
	*temp = 0; //rax
	temp -= 1;
	*temp = 0; //rbx
	temp -= 1;
	*temp = 0; //rcx
	temp -= 1;
	*temp = (uint64_t) size; //rdx
	temp -= 1;
	*temp = fd; //rdi
	temp -= 1;
	*temp = (uint64_t) buffer; //rsi
	temp -= 1;
	*temp = 0; //rbp
	temp -= 1;
	*temp = 0; //r8
	temp -= 1;
	*temp = 0; //r9
	temp -= 1;
	*temp = 0; //r10
	temp -= 1;
	*temp = 0; //r11
	temp -= 1;
	*temp = 0; //r12
	temp -= 1;
	*temp = 0; //r13
	temp -= 1;
	*temp = 0; //r14
	temp -= 1;
	*temp = 0; //r15
	temp -= 1;
	*temp = KERNEL_DATA; //ds
	temp -= 1;
	*temp = KERNEL_DATA; //es
	temp -= 1;
	*temp = KERNEL_DATA; //fs
	temp -= 1;
	*temp = KERNEL_DATA; //gs
	task->state.kernel_rsp = (uint64_t) temp;
//	printf("task: %p\n", task->state.kernel_rsp);
}

//this is actually read, hard to refactor, a mess it is
void temp_init_kernel_stack_write(uint64_t rsp, task_struct_t *task, int fd,
		void *buffer, uint64_t size) {
	uint64_t *temp = (uint64_t *) rsp;
	*temp = KERNEL_DATA; //ss
	temp -= 1;
	*temp = task->state.rsp; //rsp
	temp -= 1;
	*temp = task->state.flags; //flags
	temp -= 1;
	*temp = KERNEL_CODE; //cs
	temp -= 1;
	*temp = task->state.rip; //ip
	temp -= 1;
	*temp = 0; //rax
	temp -= 1;
	*temp = 0; //rbx
	temp -= 1;
	*temp = 0; //rcx
	temp -= 1;
	*temp = (uint64_t) size; //rdx
	temp -= 1;
	*temp = fd; //rdi
	temp -= 1;
	*temp = (uint64_t) buffer; //rsi
	temp -= 1;
	*temp = 0; //rbp
	temp -= 1;
	*temp = 0; //r8
	temp -= 1;
	*temp = 0; //r9
	temp -= 1;
	*temp = 0; //r10
	temp -= 1;
	*temp = 0; //r11
	temp -= 1;
	*temp = 0; //r12
	temp -= 1;
	*temp = 0; //r13
	temp -= 1;
	*temp = 0; //r14
	temp -= 1;
	*temp = 0; //r15
	temp -= 1;
	*temp = KERNEL_DATA; //ds
	temp -= 1;
	*temp = KERNEL_DATA; //es
	temp -= 1;
	*temp = KERNEL_DATA; //fs
	temp -= 1;
	*temp = KERNEL_DATA; //gs
	task->state.kernel_rsp = (uint64_t) temp;
//	printf("task: %p\n", task->state.kernel_rsp);
}
