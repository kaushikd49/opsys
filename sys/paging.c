#include <sys/defs.h>
#include <sys/sbunix.h>
#define PD_SIZE 512
#define PT_SIZE 512
#define PML_SIZE 512
#define PDPTR_SIZE 512
#define ULONG_ONE ((unsigned long)1) // without this left shift  more than 31 will not work
#define ULONG_ZERO ((unsigned long)0)
#define page_frame_number(PAGE_ADDRESS) (PAGE_ADDRESS>>12)
#define MAX_NUMBER_PAGES (1<<18)
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

uint64_t virtual_physfree = 0; // will be set later
uint64_t virtual_physbase = (uint64_t) VIRTUAL_PHYSBASE;
uint64_t * pml_base_ptr = NULL; // equivalent to CR3
char* free_list = NULL;
uint64_t free_list_location = ULONG_ZERO;

extern uint64_t BASE_CURSOR_POS;
extern uint64_t TIMER_LOC;
extern uint64_t glyph_pos;
//idea from linux page table
typedef struct page_t {
	char is_free;
	uint32_t ref_count;//number of references to this page
	void * virtual_address;
}page_t;
struct smap_t {
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed));

void create_free_list(uint32_t* modulep, char *free_list);
void create_free_list_test(uint32_t* modulep, page_t *free_list);
uint64_t get_free_page(char *free_list);
int check_boolarray_index(char *array, uint64_t pos, uint64_t limit);
int check_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
void clear_boolarray_index(char *array, uint64_t pos, uint64_t limit);
void clear_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
uint64_t get_free_pages(char *free_list,int order);
void return_page(uint64_t page, char *free_list);
uint64_t * get_free_frame();
uint64_t set_bit(uint64_t ele, int bit_num, int bit_val);
// todo: duplicated smap definition from main.

void create_free_list(uint32_t* modulep, char *free_list) {
	uint64_t i;
	uint64_t current_index;
	uint64_t current_bit;
	for (i = 0; i < MAX_FREELIST_LENGTH; i++) //char
		free_list[i] = 0;
//	printf("\nfree list size%d", i);
	struct smap_t* smap;
	for (smap = (struct smap_t*) (modulep + 2);
			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
			++smap) {
		if (smap->type == 1 /* memory */&& smap->length != 0) {
			uint64_t start = ((smap->base) + PAGE_ALIGN - 1)
					& ~(PAGE_ALIGN - 1);
			uint64_t page_index = start>>12;
			current_index = (page_index >> 3); //right shift 6 for the index
			current_bit = (page_index & 0xff);
			printf("Available Physical Memory [%x-%x]\n", smap->base,
																smap->base + smap->length);
			printf("%x -- %x\n", current_index, current_bit);
			while (start < (smap->base + smap->length)) {
//				printf("\n%x", start);
				if (start + 0x1000 < (smap->base + smap->length))
					free_list[current_index] |= 1 << current_bit;
				current_bit++;
				if (current_bit == 8) {
					current_index += 1;
					current_bit = 0;
				}
				start = start + 0x1000;
			}
		}
	}
	free_list[0] = free_list[0] & 0xFE;
}
/*this function has not been completed yet, we will get to it as and when we need it.*/
void create_free_list_test(uint32_t* modulep, page_t *free_list) {
	uint64_t i;
//	uint64_t current_index;
	for (i = 0; i < MAX_NUMBER_PAGES; i++){
		free_list[i].is_free = 0;
		free_list[i].ref_count = 0;
	}
//	printf("\nfree list size%d", i);
//	struct smap_t* smap;
//	for (smap = (struct smap_t*) (modulep + 2);
//			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
//			++smap) {
//		if (smap->type == 1 /* memory */&& smap->length != 0) {
//			uint64_t start = ((smap->base) + PAGE_ALIGN - 1)
//					& ~(PAGE_ALIGN - 1);
//
//			while (start < (smap->base + smap->length)) {
////				printf("\n%x", start);
//
//				current_index = (start >> 12); //right shift 12 for the page index
//				if (start + 0x1000 < (smap->base + smap->length))
//					free_list[current_index].is_free = 1;
//				start = start + 0x1000;
//			}
//		}
//	}
//	free_list[0].is_free = 0;
}
uint64_t get_free_page(char *free_list) {
	return get_free_pages(free_list, 0);
//	uint64_t k, i = 0, check = 0; //todo: could optimize to search from previous page given
//	for (; i < MAX_FREELIST_LENGTH; i++) {
//		if ((uint64_t) (free_list[i]) > 0) {
//			k = 0;
//			while (k < 8 && (uint64_t) (((1 << k) & free_list[i])) == 0) {//k < 8 is an unnecessary check
//				k++;
//			}
//			if (k < 8) {
//				check = 1;
//				break;
//			}
//		}
//	}
//	if (check == 1) {
//		uint64_t return_val = (i * 8 + k) << 12;
//		free_list[i] ^= 1 << k;
//		return return_val;
//	}
//	return 0;
}
//the pos should be a valid offset. no check done
int check_boolarray_index(char *array, uint64_t pos, uint64_t limit){
	if(pos>limit)
		return 0;
	uint64_t array_index = pos/8;
	uint64_t array_offset = pos%8;
	if((array[array_index]&(1<<array_offset))!=0)
		return 1;
	return 0;
}
//the start and end should be a valid offset. no check done
int check_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit){
	if(end>limit)
			return 0;
	uint64_t i = start;
	while(i<=end && check_boolarray_index(array, i, limit)){
		i++;
	}
	if(i >end){
		return 1;
	}
	else{
		return 0;
	}
}
void clear_boolarray_index(char *array, uint64_t pos, uint64_t limit){
	if(pos>limit)
		return;
	uint64_t array_index = pos/8;
	uint64_t array_offset = pos%8;
	char MASK = 0xFF ^ 1<<array_offset;
	array[array_index] = array[array_index]&MASK;
}
void clear_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit){
	if(end>limit)//invalid case, so if more bits are asked to be set than allowed then sets nothing
		return;
	uint64_t i = start;
	while(i<=end){
		clear_boolarray_index(array,i,limit);
		i++;
	}

}
uint64_t get_free_pages(char *free_list,int order){
	uint64_t limit = 1<<order;
	for(uint64_t i = 0; i < MAX_NUMBER_PAGES;i++){//todo: optimize
		if(check_boolarray_index(free_list,i, MAX_NUMBER_PAGES-1) == 1 && check_boolarray_index(free_list,i+limit-1, MAX_NUMBER_PAGES-1) == 1 && check_boolarray_range(free_list, i, i+limit-1, MAX_NUMBER_PAGES-1) == 1){
			clear_boolarray_range(free_list,i, i+limit-1, MAX_NUMBER_PAGES-1);
			uint64_t phys_page = i*0x1000;
			return phys_page;
		}
	}
	return 0;
}
void return_page(uint64_t page, char *free_list) {
	uint64_t page_frame = page >> 12; //remove the offset bits
	uint64_t page_index = page_frame / 8; // get the page index in the free list
	uint64_t page_shift = page_frame % 8; // get the shift in the location in the free list
	if ((uint64_t) (free_list[page_index] ^ 1 << page_shift) > 0) { //just a check to make sure that the user does not give a non-free page
		free_list[page_index] ^= 1 << page_shift;
	}
}

uint64_t * get_free_frame() {
	uint64_t freePage = get_free_page(free_list);
//	printf("returning freepage:%p ", freePage);
	return (uint64_t *) freePage;
}
uint64_t * get_free_frames(int order){
	uint64_t freePage = get_free_pages(free_list, order);
	return (uint64_t *) freePage;
}
uint64_t set_bit(uint64_t ele, int bit_num, int bit_val) {
	uint64_t bit_in_uint64 = ULONG_ONE << bit_num;
	if (bit_val == 1)
		ele |= bit_in_uint64; // set
	else
		ele &= ~bit_in_uint64; // unset
	return ele;
}

uint64_t extract_bits(uint64_t from, int fstart_bit, int fend_bit, uint64_t to,
		int tstart_bit, int tend_bit) {
	// start and end both are inclusive
	for (int i = fstart_bit, j = tstart_bit; i <= fend_bit; i++, j++) {
		uint64_t bit_in_uint64 = ULONG_ONE << i;
		unsigned int ith_bit = (from & bit_in_uint64) >> i;
		to = set_bit(to, j, ith_bit);
	}
	return to;
}

uint64_t update40bit_addr(uint64_t entry, uint64_t extract_from) {
	// 51:12 fill 40 bit address
	entry = extract_bits(extract_from, 12, 51, entry, 12, 51);
	return entry;
}

uint64_t get_pml4_entry(uint64_t pdir_ptr) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return update40bit_addr(entry, pdir_ptr);
}

uint64_t get_pdpt_entry(uint64_t pdir) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return update40bit_addr(entry, pdir);
}

uint64_t get_pd_entry(uint64_t ptable) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return update40bit_addr(entry, ptable);
}

uint64_t get_ptable_entry(uint64_t physical_addr) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return update40bit_addr(entry, physical_addr);
}

struct paging_entities {
	int page_index;
	int table_index;
	int dir_index;
	int pdir_ptr_index;
	int pml_index;
}__attribute__((packed));

// For given linear address, set various paging entities' indexes
void get_paging_entity_indexes(struct paging_entities* p_entities,
		uint64_t linear_addr) {
	p_entities->page_index = (int) extract_bits(linear_addr, 0, 11, 0, 0, 11); // may not need this
	p_entities->table_index = (int) extract_bits(linear_addr, 12, 20, 0, 0, 8);
	p_entities->dir_index = (int) extract_bits(linear_addr, 21, 29, 0, 0, 8);
	p_entities->pdir_ptr_index = (int) extract_bits(linear_addr, 30, 38, 0, 0,
			8);
	p_entities->pml_index = (int) extract_bits(linear_addr, 39, 47, 0, 0, 8);
}

uint64_t * set_paging(uint64_t linear_addr, uint64_t physical_addr) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);

	if (pml_base_ptr == NULL) {
		pml_base_ptr = get_free_frame();
	}

	uint64_t *pdir_ptr = get_free_frame();
	uint64_t *pdir = get_free_frame();
	uint64_t *ptable = get_free_frame();
//	uint64_t *page = get_free_frame();

//	printf("pdirptr %p\n", pdir_ptr);
//	printf("pdir %p\n", pdir);
//	printf("ptable %p\n", ptable);

//	printf("pml_base_ptr + pe.pml_index  %p\n", pml_base_ptr + pe.pml_index);
//	printf("pdir_ptr + pe.pdir_ptr_index  %p\n", pdir_ptr + pe.pdir_ptr_index);
//	printf("pdir + pe.dir_index %p\n", pdir + pe.dir_index);
//	printf("ptable + pe.table_index %p\n", ptable + pe.table_index);
//    printf("helloworld");
	*(pml_base_ptr + pe.pml_index) = get_pml4_entry((uint64_t) pdir_ptr);
	*(pdir_ptr + pe.pdir_ptr_index) = get_pdpt_entry((uint64_t) pdir);
	*(pdir + pe.dir_index) = get_pd_entry((uint64_t) ptable);
	*(ptable + pe.table_index) = get_ptable_entry(physical_addr);

//	printf("stored pml_base_ptr + pe.pml_index  %p\n",
//			*(pml_base_ptr + pe.pml_index));
//	printf("stored pdir_ptr + pe.pdir_ptr_index  %p\n",
//			*(pdir_ptr + pe.pdir_ptr_index));
//	printf("stored pdir + pe.dir_index %p\n", *(pdir + pe.dir_index));
//	printf("stored ptable + pe.table_index %p\n", *(ptable + pe.table_index));

	return ptable + pe.table_index;
}

int is_entry_not_created(uint64_t* entry) {
	return *entry == 0; //todo: check this logic
}

uint64_t* next_entity_base(uint64_t* entity_entry) {
	return (uint64_t *) update40bit_addr(ULONG_ZERO, *entity_entry);
}

uint64_t* next_entity_entry(uint64_t* entity_entry, int offset) {
	return next_entity_base(entity_entry) + offset;
}

// Entity - pml, pdir_ptr, pdir or ptable
// Return whether pml, pdir_ptr, pdir or ptable is not present
// Status for each would be indicated by 1,2,3,4 respectively
// If all are present, then return 0.
// Also sets the deepest entity base.
int page_lookup(uint64_t linear_addr, uint64_t* deepest_entity,
		uint64_t* deepest_entity_base) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);

	//todo: check if the sentinel 0 is ok
	*deepest_entity = *deepest_entity_base = 0;

	if (pml_base_ptr == NULL) {
//		printf("Error: pml_base_ptr is NULL");
		return -1;
	}
	uint64_t *pml = pml_base_ptr + pe.pml_index;
//	printf("pml lookup %p\n", pml);

	if (is_entry_not_created(pml)) {
		return 1;
	} else {
		*deepest_entity = (uint64_t) pml;
		// pml entry is there but its child entry is not,
		// so set its child frame's base as deepest_base
		*deepest_entity_base = (uint64_t) next_entity_base(pml);
		uint64_t* pdir_ptr = next_entity_entry(pml, pe.pdir_ptr_index);
//		printf("pdir_ptr lookup %p, deepentity:%p\n", pdir_ptr,
//				*deepest_entity);

		if (is_entry_not_created(pdir_ptr)) {
			return 2;
		} else {
			*deepest_entity = (uint64_t) pdir_ptr;
			*deepest_entity_base = (uint64_t) next_entity_base(pdir_ptr);
			uint64_t* pdir = next_entity_entry(pdir_ptr, pe.dir_index);
//			printf("pdir lookup %p, deepentity:%p\n", pdir, *deepest_entity);

			if (is_entry_not_created(pdir)) {
				return 3;
			} else {
				*deepest_entity = (uint64_t) pdir;
				*deepest_entity_base = (uint64_t) next_entity_base(pdir);
				uint64_t* ptable = next_entity_entry(pdir, pe.table_index);
//				printf("ptable lookup %p, deepentity:%p\n", ptable,
//						*deepest_entity);
				if (is_entry_not_created(ptable)) {
					return 4;
				} else {
					*deepest_entity = (uint64_t) ptable;
					*deepest_entity_base = (uint64_t) next_entity_base(pdir);
					return 0;
				}
			}
		}
	}
}

void create_all_paging_entities(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* pml_base_ptr) {
	// No entities are present for this linear address
	uint64_t* pdir_ptr = get_free_frame();
	uint64_t* pdir = get_free_frame();
	uint64_t* ptable = get_free_frame();
	*(pml_base_ptr + pe->pml_index) = get_pml4_entry((uint64_t) pdir_ptr);
	*(pdir_ptr + pe->pdir_ptr_index) = get_pdpt_entry((uint64_t) pdir);
	*(pdir + pe->dir_index) = get_pd_entry((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry(physical_addr);
}

void create_all_but_pml(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base) {
	// only pml is present for this linear address

	uint64_t* pdir_ptr_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* pdir = get_free_frame();
	uint64_t* ptable = get_free_frame();

	*(pdir_ptr_base + pe->pdir_ptr_index) = get_pdpt_entry((uint64_t) pdir);
	*(pdir + pe->dir_index) = get_pd_entry((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry(physical_addr);
}

void create_pdir_and_ptable(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base) {
	// pml and pdir_ptr are present for this linear address
	uint64_t* pdir_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* ptable = get_free_frame();

	*(pdir_base + pe->dir_index) = get_pd_entry((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry(physical_addr);
}

void create_ptable(const struct paging_entities* pe, uint64_t physical_addr,
		uint64_t* deepest_entity_base) {
	// pml, pdir_ptr and pdir are present for this linear address, but not ptable
	uint64_t* ptable_base = (uint64_t*) (*deepest_entity_base);

	*(ptable_base + pe->table_index) = get_ptable_entry(physical_addr);
}

void setup_page_tables(uint64_t linear_addr, uint64_t physical_addr) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);

	if (pml_base_ptr == NULL)
		pml_base_ptr = get_free_frame();

	// USE OF THE LOCAL VARS BELOW IS VERY IMPOROTANT.
	// USED NULL TO INITIALIZE PTRS BELOW AND CHANGE
	// OF VALUE OF ONE AFFECTED ANOTHER!
	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t * deepest_entity = &abc;
	uint64_t * deepest_entity_base = &def;
	int res = page_lookup(linear_addr, deepest_entity, deepest_entity_base);

	if (res == 1) {
		// No entities are present for this linear address
		create_all_paging_entities(&pe, physical_addr, pml_base_ptr);
	} else if (res == 2) {
		// only pml is present for this linear address
		create_all_but_pml(&pe, physical_addr, deepest_entity_base);
	} else if (res == 3) {
		// pml and pdir_ptr are present for this linear address
		create_pdir_and_ptable(&pe, physical_addr, deepest_entity_base);
	} else if (res == 4) {
		// pml, pdir_ptr and pdir are present for this linear address, but not ptable
		create_ptable(&pe, physical_addr, deepest_entity_base);
	} else {
		uint64_t* pt_base = (uint64_t *) (*deepest_entity_base);

//		printf("found page with PTE:%p, pt base:%p\n",
//				*(pt_base + pe.table_index), pt_base);

		*(pt_base + pe.table_index) = get_ptable_entry(physical_addr);

//		printf("phys addr given:%p, after re-updation %p\n", physical_addr,
//				*(pt_base + pe.table_index));
	}
}

void map_kernel_address(void* physbase, void* physfree) {
	virtual_physfree = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) physfree;
	uint64_t linear_addr = virtual_physbase;
	uint64_t physical_addr = (uint64_t) physbase;
	uint64_t range = virtual_physfree - virtual_physbase;
	uint64_t numIters = (range / 4096) + (range % 4096);
	printf("numIters needed %d\n", numIters);
	for (int i = 0; i < numIters; i++) {
		if (i == 35)
			printf("accessing %p ", linear_addr);
		setup_page_tables(linear_addr, physical_addr);
		linear_addr += 4096;
		physical_addr += 4096;
	}
	printf("fianlly mapped %p to %p", linear_addr, physfree);
}

void map_video_address() {
	setup_page_tables(VIRTUAL_ADDR_VIDMEM, 0xb8000);
	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t * deepest_entity = &abc;
	uint64_t * deepest_entity_base = &def;

	int res = page_lookup(VIRTUAL_ADDR_VIDMEM, deepest_entity,
			deepest_entity_base);
	printf("res:%d", res);
	BASE_CURSOR_POS = VIRTUAL_ADDR_VIDMEM;
	TIMER_LOC = VIRTUAL_ADDR_TIMER_LOC;
	glyph_pos = VIRTUAL_ADDR_GLYPH_POS;
}

void map_page_tables_adress() {

}
void map_other_addresses(){
	//mapping the free list
	uint64_t virtual_free_list = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list;
	uint64_t free_list_temp = (uint64_t) free_list;
	for (int i = 0; i < 3; i++){//free pages is 3 pages long
		setup_page_tables(virtual_free_list, free_list_temp);
		virtual_free_list += 4096;
		free_list_temp += 4096;
	}

	free_list =(char *)(VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list);//change the free_list address from physical to virtual
}
void map_linear_addresses(void* physbase, void* physfree) {
	map_kernel_address(physbase, physfree);
	map_video_address();
	map_other_addresses();//this function maps other random addresses we might randomly map after physfree. Example: freelist
//	map_page_tables_adress(); todo: where to map this?
}

void update_cr3() {
       uint64_t pml_base_40bits = update40bit_addr(ULONG_ZERO,
                       (uint64_t) pml_base_ptr);
       printf("pre cr3 update");
       __asm__ __volatile(
                       "cli\n\t"
                       "movq %0,%%cr3"
                       :
                       :"a"(pml_base_40bits)
               );
       printf("post cr3 update");
}
void manage_memory_test_suite(){
	printf("cr3 mapped\n");
	uint64_t ret = get_free_pages(free_list,0);
	printf("\nans1: %p", ret);
	uint64_t virtual_test_addr = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) ret;
	uint64_t test_addr = (uint64_t) ret;
	setup_page_tables(virtual_test_addr, test_addr);
	struct page_t *test = (struct page_t *)virtual_test_addr;
	test[0].is_free = 0;
	test[0].ref_count = 2;
	test[0].virtual_address = NULL;

	printf("YAYYYY:  %d", test[0].ref_count);
//	ret = get_free_pages(free_list,1);
//	printf("\nans2: %p", ret);
//	ret = get_free_pages(free_list,2);
//	printf("\nans3: %p", ret);
//	ret = get_free_pages(free_list,3);
//	printf("\nans3: %p", ret);
//	ret = get_free_pages(free_list,4);
//	printf("\nans3: %p", ret);
//
//	ret = get_free_pages(free_list,5);
//	printf("\nans3: %p", ret);
//	ret = get_free_pages(free_list,6);
//	printf("\nans3: %p", ret);
//	uint64_t *ret2 = get_free_frames(2);
//	printf("\n%x", ret2);
}
void manage_memory(void* physbase, void* physfree, uint32_t* modulep) {
// kernel starts here
//	printf("\ncreating free list");
//	uint64_t temp = (uint64_t) physfree;
//	printf("\nlocation of physfree+temp: %x", temp);

	if (free_list_location == ULONG_ZERO) {
		free_list_location = (uint64_t) ((((uint64_t) physfree)
				& (~(PAGE_SIZE - 1))) + (PAGE_SIZE)); //location 229000,
		printf("\nlocation of free list: %x\n", free_list_location);
	}
	if (free_list == NULL) {
		free_list = (char*) (free_list_location);
	}
	create_free_list(modulep, free_list);
	/*more detailed page frame structures, we should develop this more as and when we need it for now the bit array is enough*/
	uint64_t after_free_list = (uint64_t)(&free_list[MAX_FREELIST_LENGTH]);
	printf("\nlocation of after free list: %x", after_free_list);
	uint64_t after_free_list_aligned = (uint64_t) ((((uint64_t) after_free_list)
			& (~(PAGE_SIZE - 1))) + (PAGE_SIZE));//location 232000
	printf("\nlocation of after free list aligned: %x", after_free_list_aligned);
//	page_t *free_page_list = (page_t *)after_free_list_aligned;
//	create_free_list_test(modulep, free_page_list);
//	uint64_t page_frames_size = sizeof(struct page_t)*MAX_NUMBER_PAGES;
////	uint64_t kmalloc_start = after_free_list+page_frames_size;




	map_linear_addresses(physbase, physfree);
	update_cr3();
	manage_memory_test_suite();
}

