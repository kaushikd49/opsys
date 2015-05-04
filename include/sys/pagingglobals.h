#ifndef PAGINGGLOBALS_H
#define PAGINGGLOBALS_H

#define PD_SIZE 512
#define PT_SIZE 512
#define PML_SIZE 512
#define PDPTR_SIZE 512
#define ULONG_ONE ((unsigned long)1) // without this left shift  more than 31 will not work
#define ULONG_ZERO ((unsigned long)0)
#define page_frame_number(PAGE_ADDRESS) (PAGE_ADDRESS>>12)
#define PAGE_SIZE (1<<12)
#define PAGE_ALIGN (1<<12)
#define CHAR_SIZE (1<<3)
#define MAX_FREELIST_LENGTH (MAX_NUMBER_PAGES/CHAR_SIZE)
#define NULL1 ((void*)1)
#define NULL2 ((void*)2)

#define VIRTUAL_PHYSBASE 0xffffffff80200000
#define VIRTUAL_PHYSFREE_OFFSET 0xffffffff80000000
#define VIRTUAL_ADDR_VIDMEM 0xffffffff800b8000
#define VIRTUAL_ADDR_TIMER_LOC 0xffffffff800b8f80
#define VIRTUAL_ADDR_GLYPH_POS 0xffffffff800b8f60

typedef struct page_t {
	uint32_t is_free;
	uint64_t frame_addr;
	uint32_t ref_count;//number of references to this page
}page_t;
typedef struct physical_map_node{
	uint64_t start;
	uint64_t end;
}physical_map_node;
extern uint64_t virtual_physfree;
extern uint64_t virtual_physbase;
extern uint64_t * pml_base_ptr;
extern page_t * free_list;
extern uint64_t *free_list_location;

extern uint64_t BASE_CURSOR_POS;
extern uint64_t TIMER_LOC;
extern uint64_t glyph_pos;
//idea from linux page table
//PAGE FRAME variables
extern uint64_t MAX_NUMBER_PAGES;
struct smap_t {
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed));
typedef struct cache_init{
	struct cache_t *head;

	void *current_virtual;// dont care about reusing virtual space if something gets deallocated then so be it(NOTE: Physical memory is reclaimed).
}cache_init;
typedef struct cache_t{
	int order;
	int count;
	void *free;
	//cache_page *head;

}cache_t;
typedef struct free_t{
	void *next_free;
}free_t;
typedef struct kmalloc_t{
	uint64_t order;
}kmalloc_t;
void create_free_list(uint32_t* modulep, char *free_list);
void create_free_list_test(uint32_t* modulep, page_t *free_list, void *physbase, void *physfree);
uint64_t get_free_page(page_t *free_list);
int check_boolarray_index(char *array, uint64_t pos, uint64_t limit);
int check_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
void clear_boolarray_index(char *array, uint64_t pos, uint64_t limit);
void clear_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
uint64_t get_free_pages(page_t *free_list,int order);
void return_page(uint64_t page, page_t *free_list);
uint64_t * get_free_frame();
uint64_t set_bit(uint64_t ele, int bit_num, int bit_val);
uint64_t * get_physical_pml4_base_for_process();
uint64_t update40bit_addr(uint64_t entry, uint64_t extract_from);
void blank_space_baby(page_t *free_list);
// We will be using self-referential pages trick, so the entry 0 of PML will map to entry 0.
// This implies we cannot use PMLE # 0 for general purpose mapping. Corresponding bits are
// bits [39:47] which cannot be 000000000. At min, they can be 000000001 and this
// means that min virtual address can be 0000-0010-0000-0000. Make sure this is not breached.

#endif
