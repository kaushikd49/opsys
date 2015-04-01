#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/pagingglobals.h>
#include <sys/freelist.h>

#define NUM_UNIT64_IN_PAGE PAGE_SIZE/sizeof(uint64_t)

// This is used to copy apt offsets from linear addr and traverse page tables
// The key is usage of all 1's to make use of self-referential trick.
#define PG_TRVRSE_BASIC_VA_ADDR 0x0000000000000000

// todo: duplicated smap definition from main.
extern uint64_t virtual_physfree;
extern uint64_t virtual_physbase;
//uint64_t * pml_base_ptr = NULL; // equivalent to CR3
//end of free page management
uint64_t set_bit_process(uint64_t ele, int bit_num, int bit_val) {
	uint64_t bit_in_uint64 = ULONG_ONE << bit_num;
	if (bit_val == 1)
		ele |= bit_in_uint64; // set
	else
		ele &= ~bit_in_uint64; // unset
	return ele;
}

uint64_t extract_bits_process(uint64_t from, int fstart_bit, int fend_bit, uint64_t to,
		int tstart_bit, int tend_bit) {
	// start and end both are inclusive
	for (int i = fstart_bit, j = tstart_bit; i <= fend_bit; i++, j++) {
		uint64_t bit_in_uint64 = ULONG_ONE << i;
		unsigned int ith_bit = (from & bit_in_uint64) >> i;
		to = set_bit_process(to, j, ith_bit);
	}
	return to;
}

uint64_t update40bit_addr_process(uint64_t entry, uint64_t extract_from) {
	// 51:12 fill 40 bit address
	entry = extract_bits_process(extract_from, 12, 51, entry, 12, 51);
	return entry;
}

uint64_t get_pml4_entry_process(uint64_t pdir_ptr) {
	uint64_t entry = set_bit_process(ULONG_ZERO, 0, 1); // Present
	entry = set_bit_process(entry, 1, 1); // Read/Write
	return update40bit_addr_process(entry, pdir_ptr);
}

uint64_t get_pdpt_entry_process(uint64_t pdir) {
	uint64_t entry = set_bit_process(ULONG_ZERO, 0, 1); // Present
	entry = set_bit_process(entry, 1, 1); // Read/Write
	return update40bit_addr_process(entry, pdir);
}

uint64_t get_pd_entry_process(uint64_t ptable) {
	uint64_t entry = set_bit_process(ULONG_ZERO, 0, 1); // Present
	entry = set_bit_process(entry, 1, 1); // Read/Write
	return update40bit_addr_process(entry, ptable);
}

uint64_t get_ptable_entry_process(uint64_t physical_addr) {
	uint64_t entry = set_bit_process(ULONG_ZERO, 0, 1); // Present
	entry = set_bit_process(entry, 1, 1); // Read/Write
	return update40bit_addr_process(entry, physical_addr);
}

struct paging_entities {
	int page_index;
	int table_index;
	int dir_index;
	int pdir_ptr_index;
	int pml_index;
}__attribute__((packed));

// For given linear address, set various paging entities' indexes
void get_paging_entity_indexes_process(struct paging_entities* p_entities,
		uint64_t linear_addr) {
	p_entities->page_index = (int) extract_bits_process(linear_addr, 0, 11, 0, 0, 11); // may not need this
	p_entities->table_index = (int) extract_bits_process(linear_addr, 12, 20, 0, 0, 8);
	p_entities->dir_index = (int) extract_bits_process(linear_addr, 21, 29, 0, 0, 8);
	p_entities->pdir_ptr_index = (int) extract_bits_process(linear_addr, 30, 38, 0, 0,
			8);
	p_entities->pml_index = (int) extract_bits_process(linear_addr, 39, 47, 0, 0, 8);
}

int is_entry_not_created_process(uint64_t* entry) {
	return *entry == 0; //todo: check this logic
}

uint64_t* next_entity_base_process(uint64_t* entity_entry) {
	return (uint64_t *) update40bit_addr_process(ULONG_ZERO, *entity_entry);
}

uint64_t* next_entity_entry_process(uint64_t* entity_entry, int offset) {
	return next_entity_base_process(entity_entry) + offset;
}

// Entity - pml, pdir_ptr, pdir or ptable
// Return whether pml, pdir_ptr, pdir or ptable is not present
// Status for each would be indicated by 1,2,3,4 respectively
// If all are present, then return 0.
// Also sets the deepest entity base.
int page_lookup_process(uint64_t linear_addr, uint64_t* deepest_entity,
		uint64_t* deepest_entity_base, uint64_t *pml_base_ptr_process) {
	struct paging_entities pe;
	get_paging_entity_indexes_process(&pe, linear_addr);

	//todo: check if the sentinel 0 is ok
	*deepest_entity = *deepest_entity_base = 0;

	if (pml_base_ptr_process == NULL) {
//		printf("Error: pml_base_ptr_process is NULL");
		return -1;
	}
	uint64_t *pml = pml_base_ptr_process + pe.pml_index;
//	printf("pml lookup %p\n", pml);

	if (is_entry_not_created_process(pml)) {
		return 1;
	} else {
		*deepest_entity = (uint64_t) pml;
		// pml entry is there but its child entry is not,
		// so set its child frame's base as deepest_base
		*deepest_entity_base = (uint64_t) next_entity_base_process(pml);
		uint64_t* pdir_ptr = next_entity_entry_process(pml, pe.pdir_ptr_index);
//		printf("pdir_ptr lookup %p, deepentity:%p\n", pdir_ptr,
//				*deepest_entity);

		if (is_entry_not_created_process(pdir_ptr)) {
			return 2;
		} else {
			*deepest_entity = (uint64_t) pdir_ptr;
			*deepest_entity_base = (uint64_t) next_entity_base_process(pdir_ptr);
			uint64_t* pdir = next_entity_entry_process(pdir_ptr, pe.dir_index);
//			printf("pdir lookup %p, deepentity:%p\n", pdir, *deepest_entity);

			if (is_entry_not_created_process(pdir)) {
				return 3;
			} else {
				*deepest_entity = (uint64_t) pdir;
				*deepest_entity_base = (uint64_t) next_entity_base_process(pdir);
				uint64_t* ptable = next_entity_entry_process(pdir, pe.table_index);
//				printf("ptable lookup %p, deepentity:%p\n", ptable,
//						*deepest_entity);
				if (is_entry_not_created_process(ptable)) {
					return 4;
				} else {
					*deepest_entity = (uint64_t) ptable;
					*deepest_entity_base = (uint64_t) next_entity_base_process(pdir);
					return 0;
				}
			}
		}
	}
}

void create_all_paging_entities_process(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* pml_base_ptr_process) {
	// No entities are present for this linear address
	uint64_t* pdir_ptr = get_free_frame();
	uint64_t* pdir = get_free_frame();
	uint64_t* ptable = get_free_frame();
	*(pml_base_ptr_process + pe->pml_index) = get_pml4_entry_process((uint64_t) pdir_ptr);
	*(pdir_ptr + pe->pdir_ptr_index) = get_pdpt_entry_process((uint64_t) pdir);
	*(pdir + pe->dir_index) = get_pd_entry_process((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry_process(physical_addr);
}

void create_all_but_pml_process(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base) {
	// only pml is present for this linear address

	uint64_t* pdir_ptr_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* pdir = get_free_frame();
	uint64_t* ptable = get_free_frame();

	*(pdir_ptr_base + pe->pdir_ptr_index) = get_pdpt_entry_process((uint64_t) pdir);
	*(pdir + pe->dir_index) = get_pd_entry_process((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry_process(physical_addr);
}

void create_pdir_and_ptable_process(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base) {
	// pml and pdir_ptr are present for this linear address
	uint64_t* pdir_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* ptable = get_free_frame();

	*(pdir_base + pe->dir_index) = get_pd_entry_process((uint64_t) ptable);
	*(ptable + pe->table_index) = get_ptable_entry_process(physical_addr);
}

void create_ptable_process(const struct paging_entities* pe, uint64_t physical_addr,
		uint64_t* deepest_entity_base) {
	// pml, pdir_ptr and pdir are present for this linear address, but not ptable
	uint64_t* ptable_base = (uint64_t*) (*deepest_entity_base);

	*(ptable_base + pe->table_index) = get_ptable_entry_process(physical_addr);
}

void setup_page_tables_process(uint64_t linear_addr, uint64_t physical_addr, uint64_t *pml_base_ptr_process) {
	struct paging_entities pe;
	get_paging_entity_indexes_process(&pe, linear_addr);

	if (pml_base_ptr_process == NULL)
		pml_base_ptr_process = get_free_frame();

	// USE OF THE LOCAL VARS BELOW IS VERY IMPOROTANT.
	// USED NULL TO INITIALIZE PTRS BELOW AND CHANGE
	// OF VALUE OF ONE AFFECTED ANOTHER!
	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t * deepest_entity = &abc;
	uint64_t * deepest_entity_base = &def;
	int res = page_lookup_process(linear_addr, deepest_entity, deepest_entity_base,pml_base_ptr_process);

	if (res == 1) {
		// No entities are present for this linear address
		create_all_paging_entities_process(&pe, physical_addr, pml_base_ptr_process);
	} else if (res == 2) {
		// only pml is present for this linear address
		create_all_but_pml_process(&pe, physical_addr, deepest_entity_base);
	} else if (res == 3) {
		// pml and pdir_ptr are present for this linear address
		create_pdir_and_ptable_process(&pe, physical_addr, deepest_entity_base);
	} else if (res == 4) {
		// pml, pdir_ptr and pdir are present for this linear address, but not ptable
		create_ptable_process(&pe, physical_addr, deepest_entity_base);
	} else {
		uint64_t* pt_base = (uint64_t *) (*deepest_entity_base);

//		printf("found page with PTE:%p, pt base:%p\n",
//				*(pt_base + pe.table_index), pt_base);

		*(pt_base + pe.table_index) = get_ptable_entry_process(physical_addr);

//		printf("phys addr given:%p, after re-updation %p\n", physical_addr,
//				*(pt_base + pe.table_index));
	}
}

void map_kernel_address_process(void* physbase, void* physfree, uint64_t *pml_base_ptr_process) {
	virtual_physfree = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) physfree;
	uint64_t linear_addr = virtual_physbase;
	uint64_t physical_addr = (uint64_t) physbase;
	uint64_t range = virtual_physfree - virtual_physbase;
	uint64_t numIters = (range / 4096) + (range % 4096);
//	printf("numIters needed %d\n", numIters);
	for (int i = 0; i < numIters; i++) {
//		if (i == 35)
//			printf("accessing %p ", linear_addr);
		setup_page_tables_process(linear_addr, physical_addr, pml_base_ptr_process);
		linear_addr += 4096;
		physical_addr += 4096;
	}
//	printf("fianlly mapped %p to %p", linear_addr, physfree);
}

void map_video_address_process(uint64_t *pml_base_ptr_process) {
	setup_page_tables_process(VIRTUAL_ADDR_VIDMEM, 0xb8000, pml_base_ptr_process);
	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t * deepest_entity = &abc;
	uint64_t * deepest_entity_base = &def;

	int res = page_lookup_process(VIRTUAL_ADDR_VIDMEM, deepest_entity,
			deepest_entity_base, pml_base_ptr_process);
	printf("res:%d", res);
	BASE_CURSOR_POS = VIRTUAL_ADDR_VIDMEM;
	TIMER_LOC = VIRTUAL_ADDR_TIMER_LOC;
	glyph_pos = VIRTUAL_ADDR_GLYPH_POS;
}

uint64_t copy_as_pageoffset_process(uint64_t linear_addr, int startbit, int endbit,
		uint64_t copy_into) {
	uint64_t temp = extract_bits_process(linear_addr, startbit, endbit,
	PG_TRVRSE_BASIC_VA_ADDR, 0, 8) * 8;
	uint64_t res = extract_bits_process(temp, 0, 11, copy_into, 0, 11);
	return res;
}

uint64_t * virtual_addr_pml4e_process(uint64_t linear_addr) {
	// copy the pml4 offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR as the page offset.
	uint64_t pml4e_virtual_addr = copy_as_pageoffset_process(linear_addr, 39, 47,
	PG_TRVRSE_BASIC_VA_ADDR);
	return (uint64_t *) pml4e_virtual_addr;
}

uint64_t * virtual_addr_pdirptre_process(uint64_t linear_addr) {
	// copy the pml offset and pdirptr offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
	//as table and page offsets respectively.
	uint64_t temp = extract_bits_process(linear_addr, 39, 47,
	PG_TRVRSE_BASIC_VA_ADDR, 12, 20);
	uint64_t pdir_ptre_virtual_addr = copy_as_pageoffset_process(linear_addr, 30, 38,
			temp);
	return (uint64_t *) pdir_ptre_virtual_addr;
}

uint64_t * virtual_addr_pdire_process(uint64_t linear_addr) {
	// copy pml, pdirptr and dir offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
	// as dir, table and page offsets respectively
	uint64_t temp = extract_bits_process(linear_addr, 39, 47, PG_TRVRSE_BASIC_VA_ADDR,
			21, 29);
	temp = extract_bits_process(linear_addr, 30, 38, temp, 12, 20);
	uint64_t pdire_virtual_addr = copy_as_pageoffset_process(linear_addr, 21, 29, temp);
	return (uint64_t *) pdire_virtual_addr;
}

uint64_t * virtual_addr_pte_process(uint64_t linear_addr) {
	// copy pml, pdirptr, dir and table offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
	// as dirptr, dir, table and page offsets respectively
	uint64_t temp = extract_bits_process(linear_addr, 39, 47, PG_TRVRSE_BASIC_VA_ADDR,
			30, 38);
	temp = extract_bits_process(linear_addr, 30, 38, temp, 21, 29);
	temp = extract_bits_process(linear_addr, 21, 29, temp, 12, 20);
	uint64_t pte_virtual_addr = copy_as_pageoffset_process(linear_addr, 12, 20, temp);
	return (uint64_t *) pte_virtual_addr;
}

//todo: verify this is working and use cleanup in virtual addr space
void cleanup_page_process(uint64_t * page_table_entity_entry) {
	uint64_t page_table_entity_base = extract_bits_process(
			(uint64_t) page_table_entity_entry, 12, 63, ULONG_ZERO, 12, 63);
//	printf("\n input:%p, cleanup from %p:", page_table_entity_entry,
//			page_table_entity_base);
	uint64_t * ptr = (uint64_t *) page_table_entity_base;
	for (int i = 0; i < NUM_UNIT64_IN_PAGE; i++, ptr++) {
//		printf("clearing %p ", ptr);
		*ptr = 0;
	}
//	printf("%p done %p", page_table_entity_base, ptr);
}

static inline void invalidate_tlb_process(uint64_t* m) {
	__asm__ __volatile__ ( "invlpg (%0)" : : "b"(m) : "memory" );
}


void invalidate_addresses_with_page_process(uint64_t * virtual_addr) {
	uint64_t temp = extract_bits_process((uint64_t) virtual_addr, 12, 63, ULONG_ZERO,
			12, 63);
	// there are 4096 linear addresses whose page = that of this virtual addr
	// and all of them need to be flushed
	for (uint64_t i = 0; i < PAGE_SIZE; i++) {
		uint64_t addr = extract_bits_process(i, 0, 11, temp, 0, 11);
//		printf("clearing %p ", addr);
		invalidate_tlb_process((uint64_t *)addr);
	}
}

void setup_page_tables_after_cr3_update_process(uint64_t linear_addr,
		uint64_t physical_addr) {
	// derive paging entities from linear address and update their target
//	printf("\nlinear_addr is %p, physical:%p\n", linear_addr, physical_addr);
	uint64_t * pml4e_virtual_addr = virtual_addr_pml4e_process(linear_addr);
	if (is_entry_not_created_process((uint64_t *) pml4e_virtual_addr)) {
		uint64_t* pdir_ptr = get_free_frame();
		*pml4e_virtual_addr = get_pml4_entry_process((uint64_t) pdir_ptr);
//		uint64_t* virtualAddrPdirptre = virtual_addr_pdirptre_process(linear_addr);
//		cleanup_page_process(virtualAddrPdirptre);
	}
//	printf("\npml4e_virtual_addr:%p, value:%p\n", pml4e_virtual_addr,
//			*pml4e_virtual_addr);

	uint64_t * pdir_ptre_virtual_addr = virtual_addr_pdirptre_process(linear_addr);
	if (is_entry_not_created_process((uint64_t *) pdir_ptre_virtual_addr)) {
		uint64_t* pdir = get_free_frame();
		*pdir_ptre_virtual_addr = get_pdpt_entry_process((uint64_t) pdir);
	}
//	printf("\npdir_ptre_virtual_addr:%p, value:%p\n", pdir_ptre_virtual_addr,
//			*pdir_ptre_virtual_addr);

	uint64_t * pdire_virtual_addr = virtual_addr_pdire_process(linear_addr);
	if (is_entry_not_created_process((uint64_t *) pdire_virtual_addr)) {
		uint64_t* ptable = get_free_frame();
		*pdire_virtual_addr = get_pd_entry_process((uint64_t) ptable);
	}
//	printf("\npdire_virtual_addr:%p, value:%p\n", pdire_virtual_addr,
//			*pdire_virtual_addr);

	uint64_t * pte_virtual_addr = virtual_addr_pte_process(linear_addr);
	int present_before = !(is_entry_not_created_process((uint64_t *) pte_virtual_addr));
	*pte_virtual_addr = get_ptable_entry_process(physical_addr);
	if (present_before) {
		// invalidate TLB entry
//		printf("!!!invalidating TLB!!!");
		invalidate_addresses_with_page_process((uint64_t *) linear_addr);
	}
//	printf("\npte_virtual_addr:%p, value:%p\n", pte_virtual_addr,
//			*pte_virtual_addr);
}

// IMPORTANT - do not use virtual addresses corresponding to 0 PML offset
void map_page_tables_adress_process(uint64_t *pml_base_ptr_process) {
	// Make the first entry of pml4 point to the itself
	*pml_base_ptr_process = get_pml4_entry_process((uint64_t) pml_base_ptr_process);
//	printf("*pml_base_ptr_process is %p\n", *pml_base_ptr_process);
}

void map_other_addresses_process(uint64_t *pml_base_ptr_process) {
	//mapping the free list
	uint64_t virtual_free_list = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list;
	uint64_t free_list_temp = (uint64_t) free_list;
	for (int i = 0; i < 3; i++) {	//free pages is 3 pages long
		setup_page_tables_process(virtual_free_list, free_list_temp, pml_base_ptr_process);
		virtual_free_list += 4096;
		free_list_temp += 4096;
	}

	free_list = (page_t *) (VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list);//change the free_list address from physical to virtual
}

void map_linear_addresses_process(void* physbase, void* physfree, uint64_t *pml_base_ptr_process) {
	map_kernel_address_process(physbase, physfree, pml_base_ptr_process);
	map_video_address_process(pml_base_ptr_process);
	map_other_addresses_process(pml_base_ptr_process);//this function maps other random addresses we might randomly map after physfree. Example: freelist
	map_page_tables_adress_process(pml_base_ptr_process);

}

void update_cr3_process(uint64_t *pml_base_ptr_process) {
	uint64_t pml_base_40bits = update40bit_addr_process(ULONG_ZERO,
			(uint64_t) pml_base_ptr_process);
//	printf("pre cr3 update");
	__asm__ __volatile(
			"cli\n\t"
			"movq %0,%%cr3"
			:
			:"a"(pml_base_40bits)
	);
//	printf("post cr3 update");
}
void manage_memory_test_suite_process() {
//	printf("cr3 mapped\n");
	uint64_t *ret = get_free_frames(0);
//	printf("\nans1: %p", ret);
	uint64_t virtual_test_addr = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) ret;
	uint64_t test_addr = (uint64_t) ret;
	setup_page_tables_after_cr3_update_process(virtual_test_addr, test_addr);
	*((uint64_t *) virtual_test_addr) = 0xb00b;
//	printf("\nmem access after ptable-setup %x",
//			*((uint64_t *) virtual_test_addr));
	setup_page_tables_after_cr3_update_process(virtual_test_addr, test_addr + 5000);
//	printf(" mem access after ptable-setup %x",
//			*((uint64_t *) virtual_test_addr));
	setup_page_tables_after_cr3_update_process(0x1234455355, test_addr);
//	printf(" newwww mem access after ptable-setup %x",
//			*((uint64_t *) 0x1234455355));

	//	struct page_t *test = (struct page_t *)virtual_test_addr;
//	test[0].is_free = 0;
//	test[0].ref_count = 2;
//	test[0].virtual_address = NULL;
//	printf("YAYYYY:  %d", test[0].ref_count);
//	ret = get_free_frames(1);
//	printf("\nans2: %p", ret);
//	ret = get_free_frames(2);
//	printf("\nans3: %p", ret);
//	ret = get_free_frames(3);
//	printf("\nans3: %p", ret);
//	ret = get_free_frames(4);
//	printf("\nans3: %p", ret);
//
//	ret = get_free_frames(5);
//	printf("\nans3: %p", ret);
//	ret = get_free_frames(6);
//	printf("\nans3: %p", ret);
//	uint64_t *ret2 = get_free_frames(2);
//	printf("\n%x", ret2);
}

void do_paging_process(void* physbase, void* physfree, uint64_t *pml_base_ptr_process) {
	map_linear_addresses_process(physbase, physfree, pml_base_ptr_process);
	//update_cr3_process(pml_base_ptr_process);
}

void manage_memory_process(void* physbase, void* physfree, uint32_t* modulep, uint64_t *ptr_base_ptr_process) {
// kernel starts here
//	printf("\ncreating free list");
//	uint64_t temp = (uint64_t) physfree;
//	printf("\nlocation of physfree+temp: %x", temp);

	if (free_list_location == NULL) {
		free_list_location = (uint64_t *) ((((uint64_t) physfree)
				& (~(PAGE_SIZE - 1))) + (PAGE_SIZE)); //location 229000,
//		printf("\nlocation of free list: %x\n", free_list_location);
	}
	if (free_list == NULL) {
		free_list = (page_t *) (free_list_location);
	}
	create_free_list_test(modulep, free_list, physbase, physfree);
	//can write a better check below. Your wish
	uint64_t number_pages_free_list = (MAX_NUMBER_PAGES * sizeof(struct page_t))
			/ PAGE_SIZE + 1; //potentially wasting a page if it exactly page sizes here but thats fine and rare
//	printf(" NUMBER OF PAGES TAKEN by FREE_PAGES %d\n", number_pages_free_list);
	for (int i = 0; i < number_pages_free_list; i++) {
		mark_frame_used((uint64_t) free_list_location + i * (0x1000));
	}
	mark_frame_used(0xb8000);
	do_paging_process(physbase, physfree,ptr_base_ptr_process);
//	manage_memory_test_suite_process();
}
