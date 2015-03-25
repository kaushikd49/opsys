#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <stdarg.h>
#include <sys/paging.h>
#include <sys/main.h>
#include <sys/isr_stuff.h>
#include <sys/initkeyboard.h>
#include <sys/printtime.h>
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
