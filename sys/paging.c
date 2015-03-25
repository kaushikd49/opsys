#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/pagingglobals.h>
#include <sys/freelist.h>
// todo: duplicated smap definition from main.
uint64_t virtual_physfree = 0; // will be set later
uint64_t virtual_physbase = (uint64_t) VIRTUAL_PHYSBASE;
uint64_t * pml_base_ptr = NULL; // equivalent to CR3
//end of free page management
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

	free_list =(page_t *)(VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list);//change the free_list address from physical to virtual
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
	uint64_t *ret = get_free_frames(0);
	printf("\nans1: %p", ret);
//	uint64_t virtual_test_addr = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) ret;
//	uint64_t test_addr = (uint64_t) ret;
//	setup_page_tables(virtual_test_addr, test_addr);
//	struct page_t *test = (struct page_t *)virtual_test_addr;
//	test[0].is_free = 0;
//	test[0].ref_count = 2;
//	test[0].virtual_address = NULL;
//	printf("YAYYYY:  %d", test[0].ref_count);
	ret = get_free_frames(1);
	printf("\nans2: %p", ret);
	ret = get_free_frames(2);
	printf("\nans3: %p", ret);
	ret = get_free_frames(3);
	printf("\nans3: %p", ret);
	ret = get_free_frames(4);
	printf("\nans3: %p", ret);

	ret = get_free_frames(5);
	printf("\nans3: %p", ret);
	ret = get_free_frames(6);
	printf("\nans3: %p", ret);
	uint64_t *ret2 = get_free_frames(2);
	printf("\n%x", ret2);
}
void manage_memory(void* physbase, void* physfree, uint32_t* modulep) {
// kernel starts here
//	printf("\ncreating free list");
//	uint64_t temp = (uint64_t) physfree;
//	printf("\nlocation of physfree+temp: %x", temp);

	if (free_list_location == NULL) {
		free_list_location = (uint64_t *) ((((uint64_t) physfree)
				& (~(PAGE_SIZE - 1))) + (PAGE_SIZE)); //location 229000,
		printf("\nlocation of free list: %x\n", free_list_location);
	}
	if (free_list == NULL) {
		free_list = (page_t *) (free_list_location);
	}
	create_free_list_test(modulep, free_list, physbase, physfree);
	//can write a better check below. Your wish
	uint64_t number_pages_free_list = (MAX_NUMBER_PAGES*sizeof(struct page_t))/PAGE_SIZE + 1;//potentially wasting a page if it exactly page sizes here but thats fine and rare
	printf(" NUMBER OF PAGES TAKEN by FREE_PAGES %d\n", number_pages_free_list);
	for(int i = 0;i<number_pages_free_list;i++){
		mark_frame_used((uint64_t)free_list_location + i*(0x1000));
	}
	mark_frame_used(0xb8000);
	map_linear_addresses(physbase, physfree);
	update_cr3();
	manage_memory_test_suite();
}

