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
void start(uint32_t* modulep, void* physbase, void* physfree) {
	while (modulep[0] != 0x9001) {
		modulep += modulep[1] + 2;
	}
//	for (smap = (struct smap_t*) (modulep + 2);
//			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
//			++smap) {
//		if (smap->type == 1 /* memory */&& smap->length != 0) {
//			printf("Length of memory:%d\n", smap->length);
//				printf("Available Physical Memory [%x-%x]\n", smap->base,
//						smap->base + smap->length);
//		}
//	}
	printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	// kernel starts here
	manage_memory(physbase, physfree, modulep);
	init_caches();
	traverse_linked_list();
	init_init_IDT();
	config_PIC();
	add_custom_interrupt();
	init_keyboard_map();
	keyboard_init();
//	pagingTests(physbase, physfree, modulep);
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
