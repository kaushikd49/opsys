#ifndef _PROCESS_H
#define _PROCESS_H
typedef struct mem_desc {
	struct vma *vma_list; //list of the memory regions as a linked list
	struct vma *vma_cache; // last memory region used
	int mm_users; //number of users using the address space, if there are 2 threads
	int mm_count; // total number of users (can be anything
	int num_vma;  //number of memory regions
	struct mem_desc *mem_desc_head; //lets optimize later, for now it is the init process

	// addresses in ELF to read from and load to mem during demand paging
	char *text_elf_addr;
	char *rodata_elf_addr;
	char *data_elf_addr;

	uint64_t start_code;
	uint64_t end_code;
	uint64_t start_data;
	uint64_t end_data;
	uint64_t start_brk;
	uint64_t brk;
	uint64_t start_stack;
	uint64_t arg_start;
	uint64_t arg_end;
	uint64_t env_start;
	uint64_t env_end;

} mem_desc_t;
//if we do CLONE_MM ( for creating threads, the process descriptor should point to the same MM
//todo: make a function to copy mm
//when a process is done with an address space, basically decrements the mm_users count.
//if the user count reaches 0 then the mm_count is reduced. if mm count reaches 0, then we can free mem_desc

typedef struct vma {
	struct mem_desc *my_mem_desc;
	uint64_t vma_start;
	uint64_t vma_end; //no overlap in the areas.
	int vm_perm; //0: read, 1: write
	int type; // 0: text,1: rodata, 2: data, 3: bss
	struct vma *vma_next;

} vma_t;
//x86 has just 8 general-purpose registers available (eax, ebx, ecx, edx, ebp, esp, esi, edi).
//x64 extended them to 64 bits (prefix "r" instead of "e") and added another 8 (r8, r9, r10, r11, r12, r13, r14, r15).
typedef struct process_state {
//	do not change order !!!!!!! in this structure very imp,
//	need to change in process_switch
	//main pointers
	uint64_t rsp;
	uint64_t rip;
	//general registers
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	//page table stuff
	uint64_t cr3;
	//flags
	uint64_t flags;
	//segment registers
	uint64_t cs;
	uint64_t ds;
	uint64_t es;
	uint64_t fs;
	uint64_t gs;
	uint64_t ss;

} process_state;
typedef struct task_struct {
	uint64_t pid;
	uint64_t ppid;
	process_state state;
	struct task_struct *next;
	struct mem_desc *mem_map;
	char executable[100];
} task_struct_t;
void load_executable(task_struct_t *currenttask);
void preempt();
void kernel_create_process(task_struct_t *task, task_struct_t *parent_task,
		char *executable);
void kernel_process_init();
#endif
