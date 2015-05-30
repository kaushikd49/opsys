#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/pagingglobals.h>
#include <sys/freelist.h>
#include <sys/kmalloc.h>
#include <sys/process.h>

#define NUM_UNIT64_IN_PAGE (PAGE_SIZE/sizeof(uint64_t))
// This is used to copy apt offsets from linear addr and
// traverse page tables. Sign extension for msb is very important.
#define PG_TRVRSE_BASIC_VA_ADDR 0xFFFFFF7FBFDFEFF0

// This is needed to copy over the kernel pml base to processes' pml base
#define KERNEL_PML4_BASE_VIRTUAL 0xFFFFFF7FBFDFE000
int is_linear_addr_mapped(uint64_t linear_addr);
void *get_virtual_location(int order);

// todo: duplicated smap definition from main.
uint64_t virtual_physfree = 0; // will be set later
uint64_t virtual_physbase = (uint64_t) VIRTUAL_PHYSBASE;
uint64_t * kernel_pml_base_ptr = NULL; // physical address
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

int get_bit(uint64_t num, int bit_num) {
	return (int) extract_bits(num, bit_num, bit_num, ULONG_ZERO, 0, 0);
}
uint64_t update40bit_addr(uint64_t entry, uint64_t extract_from) {
	// 51:12 fill 40 bit address
	entry = extract_bits(extract_from, 12, 51, entry, 12, 51);
	return entry;
}

uint64_t get_pml4_entry(uint64_t pdir_ptr, int p, int rw, int us) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, p); // Present
	entry = set_bit(entry, 1, rw); // Read/Write
	entry = set_bit(entry, 2, us); // User/Supervisor
	return update40bit_addr(entry, pdir_ptr);
}

uint64_t get_pdpt_entry(uint64_t pdir, int p, int rw, int us) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, p); // Present
	entry = set_bit(entry, 1, rw); // Read/Write
	entry = set_bit(entry, 2, us); // User/Supervisor
	return update40bit_addr(entry, pdir);
}

uint64_t get_pd_entry(uint64_t ptable, int p, int rw, int us) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, p); // Present
	entry = set_bit(entry, 1, rw); // Read/Write
	entry = set_bit(entry, 2, us); // User/Supervisor
	return update40bit_addr(entry, ptable);
}

uint64_t get_ptable_entry(uint64_t physical_addr, int p, int rw, int us) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, p); // Present
	entry = set_bit(entry, 1, rw); // Read/Write
	entry = set_bit(entry, 2, us); // User/Supervisor
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

int is_entry_not_created(uint64_t* entry) {
	return *entry == 0; //todo: check this logic
}

uint64_t* next_entity_base(uint64_t* entity_entry) {
	return (uint64_t *) update40bit_addr(ULONG_ZERO, *entity_entry);
}

uint64_t* next_entity_base_get(uint64_t* entity_entry,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {
	return addr_map_func(next_entity_base(entity_entry), pv_map);
}

uint64_t* identity_addr_map(uint64_t* addr, void * pv_map) {
	return addr;
}
//uint64_t* next_entity_entry(uint64_t* entity_entry, int offset,
//		uint64_t* (*next_entity_base_func)(uint64_t*, void *), void * pv_map) {
//	return next_entity_base_func(entity_entry) + offset;
//}

// Entity - pml, pdir_ptr, pdir or ptable
// Return whether pml, pdir_ptr, pdir or ptable is not present
// Status for each would be indicated by 1,2,3,4 respectively
// If all are present, then return 0.
// Also sets the deepest entity base.
int page_lookup(uint64_t *pml_base_ptr, uint64_t linear_addr,
		uint64_t* deepest_entity, uint64_t* deepest_entity_base,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);

	//todo: check if the sentinel 0 is ok
	*deepest_entity = *deepest_entity_base = 0;

	if (pml_base_ptr == NULL) {
		//printf("Error: pml_base_ptr is NULL");
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
		*deepest_entity_base = (uint64_t) next_entity_base_get(pml,
				addr_map_func, pv_map);
		uint64_t* pdir_ptr = next_entity_base_get(pml, addr_map_func, pv_map)
				+ pe.pdir_ptr_index;
//		printf("pdir_ptr lookup %p, deepentity:%p\n", pdir_ptr,
//				*deepest_entity);

		if (is_entry_not_created(pdir_ptr)) {
			return 2;
		} else {
			*deepest_entity = (uint64_t) pdir_ptr;
			*deepest_entity_base = (uint64_t) next_entity_base_get(pdir_ptr,
					addr_map_func, pv_map);
			uint64_t* pdir = next_entity_base_get(pdir_ptr, addr_map_func,
					pv_map) + pe.dir_index;
//			printf("pdir lookup %p, deepentity:%p\n", pdir, *deepest_entity);

			if (is_entry_not_created(pdir)) {
				return 3;
			} else {
				*deepest_entity = (uint64_t) pdir;
				*deepest_entity_base = (uint64_t) next_entity_base_get(pdir,
						addr_map_func, pv_map);
				uint64_t* ptable = next_entity_base_get(pdir, addr_map_func,
						pv_map) + pe.table_index;
//				printf("ptable lookup %p, deepentity:%p\n", ptable,
//						*deepest_entity);
				if (is_entry_not_created(ptable)) {
					return 4;
				} else {
					*deepest_entity = (uint64_t) ptable;
					*deepest_entity_base = (uint64_t) next_entity_base_get(pdir,
							addr_map_func, pv_map);
					return 0;
				}
			}
		}
	}
}

void create_all_paging_entities(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* pml_base_ptr, int ptable_us, int p,
		int rw, int us, uint64_t* (*addr_map_func)(uint64_t*, void *),
		void * pv_map) {
	// No entities are present for this linear address
	uint64_t* pdir_ptr = get_free_zeroed_frame();
	uint64_t* pdir = get_free_zeroed_frame();
	uint64_t* ptable = get_free_zeroed_frame();

	uint64_t* mapped_pdir_ptr = addr_map_func(pdir_ptr, pv_map);
	uint64_t* mapped_pdir = addr_map_func(pdir, pv_map);
	uint64_t* mapped_ptable = addr_map_func(ptable, pv_map);

	pml_base_ptr[pe->pml_index] = get_pml4_entry((uint64_t) pdir_ptr, 1, 1,
			ptable_us);
	mapped_pdir_ptr[pe->pdir_ptr_index] = get_pdpt_entry((uint64_t) pdir, 1, 1,
			ptable_us);
	mapped_pdir[pe->dir_index] = get_pd_entry((uint64_t) ptable, 1, 1,
			ptable_us);
	mapped_ptable[pe->table_index] = get_ptable_entry(physical_addr, p, rw, us);
}

void create_all_but_pml(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base, int ptable_us,
		int p, int rw, int us, uint64_t* (*addr_map_func)(uint64_t*, void *),
		void * pv_map) {
	// only pml is present for this linear address

	uint64_t* pdir_ptr_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* pdir = get_free_zeroed_frame();
	uint64_t* ptable = get_free_zeroed_frame();

	uint64_t* mapped_pdir = addr_map_func(pdir, pv_map);
	uint64_t* mapped_ptable = addr_map_func(ptable, pv_map);

	pdir_ptr_base[pe->pdir_ptr_index] = get_pdpt_entry((uint64_t) pdir, 1, 1,
			ptable_us);
	mapped_pdir[pe->dir_index] = get_pd_entry((uint64_t) ptable, 1, 1,
			ptable_us);
	mapped_ptable[pe->table_index] = get_ptable_entry(physical_addr, p, rw, us);
}

void create_pdir_and_ptable(const struct paging_entities* pe,
		uint64_t physical_addr, uint64_t* deepest_entity_base, int ptable_us,
		int p, int rw, int us, uint64_t* (*addr_map_func)(uint64_t*, void *),
		void * pv_map) {
	// pml and pdir_ptr are present for this linear address
	uint64_t* pdir_base = (uint64_t*) (*deepest_entity_base);
	uint64_t* ptable = get_free_zeroed_frame();

	uint64_t* mapped_ptable = addr_map_func(ptable, pv_map);

	pdir_base[pe->dir_index] = get_pd_entry((uint64_t) ptable, 1, 1, ptable_us);
	mapped_ptable[pe->table_index] = get_ptable_entry(physical_addr, p, rw, us);
}

void create_ptable(const struct paging_entities* pe, uint64_t physical_addr,
		uint64_t* deepest_entity_base, int p, int rw, int us,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {
	// pml, pdir_ptr and pdir are present for this linear address, but not ptable
	uint64_t* ptable_base = (uint64_t*) (*deepest_entity_base);

	ptable_base[pe->table_index] = get_ptable_entry(physical_addr, p, rw, us);
}

void create_pte(const struct paging_entities* pe, uint64_t physical_addr,
		uint64_t* deepest_entity_base, int p, int rw, int us,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {
	uint64_t* pt_base = (uint64_t*) (*deepest_entity_base);
	//		printf("found page with PTE:%p, pt base:%p\n",
	//				*(pt_base + pe.table_index), pt_base);
	pt_base[pe->table_index] = get_ptable_entry(physical_addr, p, rw, us);
	//		printf("phys addr given:%p, after re-updation %p\n", physical_addr,
	//				*(pt_base + pe.table_index));

}

void setup_page_table_from_outside(uint64_t linear_addr, uint64_t physical_addr,
		int us, int pte_p, int pte_rw, int pte_us, uint64_t** pml_base_dbl_ptr,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);
	if (*pml_base_dbl_ptr == NULL)
		*pml_base_dbl_ptr = get_free_zeroed_frame();

	// USE OF THE LOCAL VARS BELOW IS VERY IMPOROTANT.
	// USED NULL TO INITIALIZE PTRS BELOW AND CHANGE
	// OF VALUE OF ONE AFFECTED ANOTHER!
	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t* deepest_entity = &abc;
	uint64_t* deepest_entity_base = &def;
	int res = page_lookup(*pml_base_dbl_ptr, linear_addr, deepest_entity,
			deepest_entity_base, addr_map_func, pv_map);
	if (res == 1) {
		// No entities are present for this linear address
		create_all_paging_entities(&pe, physical_addr, *pml_base_dbl_ptr, us,
				pte_p, pte_rw, pte_us, addr_map_func, pv_map);
	} else if (res == 2) {
		// only pml is present for this linear address
		create_all_but_pml(&pe, physical_addr, deepest_entity_base, us, pte_p,
				pte_rw, pte_us, addr_map_func, pv_map);
	} else if (res == 3) {
		// pml and pdir_ptr are present for this linear address
		create_pdir_and_ptable(&pe, physical_addr, deepest_entity_base, us,
				pte_p, pte_rw, pte_us, addr_map_func, pv_map);
	} else if (res == 4) {
		// pml, pdir_ptr and pdir are present for this linear address, but not ptable
		create_ptable(&pe, physical_addr, deepest_entity_base, pte_p, pte_rw,
				pte_us, addr_map_func, pv_map);
	} else {
		// all present
		create_pte(&pe, physical_addr, deepest_entity_base, pte_p, pte_rw,
				pte_us, addr_map_func, pv_map);
	}
}

void setup_kernel_page_table_from_outside(uint64_t linear_addr,
		uint64_t physical_addr, int pte_p, int pte_rw, int pte_us,
		uint64_t** pml_base_dbl_ptr,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {

	setup_page_table_from_outside(linear_addr, physical_addr, 0, pte_p, pte_rw,
			pte_us, pml_base_dbl_ptr, addr_map_func, pv_map);
}

void setup_process_page_table_from_outside(uint64_t linear_addr,
		uint64_t physical_addr, int pte_p, int pte_rw, int pte_us,
		uint64_t** pml_base_dbl_ptr,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map) {

	setup_page_table_from_outside(linear_addr, physical_addr, 1, pte_p, pte_rw,
			pte_us, pml_base_dbl_ptr, addr_map_func, pv_map);
}

void setup_page_tables(uint64_t **pml_base_dbl_ptr, uint64_t linear_addr,
		uint64_t physical_addr, int p, int rw, int us) {
	setup_kernel_page_table_from_outside(linear_addr, physical_addr, p, rw, us,
			pml_base_dbl_ptr, identity_addr_map, NULL);
}

void map_kernel_address(uint64_t **pml_base_dbl_ptr, void* physbase,
		void* physfree, int p, int rw, int us) {
	virtual_physfree = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) physfree;
	uint64_t linear_addr = virtual_physbase;
	uint64_t physical_addr = (uint64_t) physbase;
	uint64_t range = virtual_physfree - virtual_physbase;
	uint64_t numIters = (range / 4096) + (range % 4096);
//	printf("numIters needed %d\n", numIters);
	for (int i = 0; i < numIters; i++) {
//		if (i == 35)
//			printf("accessing %p ", linear_addr);
		setup_page_tables(pml_base_dbl_ptr, linear_addr, physical_addr, p, rw,
				us);
		linear_addr += 4096;
		physical_addr += 4096;
	}
//	printf("fianlly mapped %p to %p", linear_addr, physfree);
}

void map_video_address(uint64_t **pml_base_dbl_ptr, int p, int rw, int us) {
	setup_page_tables(pml_base_dbl_ptr, VIRTUAL_ADDR_VIDMEM, 0xb8000, p, rw,
			us);
//	uint64_t abc = 1;
//	uint64_t def = 2;
//	uint64_t * deepest_entity = &abc;
//	uint64_t * deepest_entity_base = &def;

//	int res = page_lookup(*pml_base_dbl_ptr, VIRTUAL_ADDR_VIDMEM,
//			deepest_entity, deepest_entity_base);
//	printf("res:%d", res);
	BASE_CURSOR_POS = VIRTUAL_ADDR_VIDMEM;
	TIMER_LOC = VIRTUAL_ADDR_TIMER_LOC;
	glyph_pos = VIRTUAL_ADDR_GLYPH_POS;
}

uint64_t copy_as_pageoffset(uint64_t linear_addr, int startbit, int endbit,
		uint64_t copy_into) {
	uint64_t temp = extract_bits(linear_addr, startbit, endbit,
	PG_TRVRSE_BASIC_VA_ADDR, 0, 8) * 8;
	uint64_t res = extract_bits(temp, 0, 11, copy_into, 0, 11);
	return res;
}

uint64_t * virtual_addr_pml4e(uint64_t linear_addr) {
// copy the pml4 offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR as the page offset.
	uint64_t pml4e_virtual_addr = copy_as_pageoffset(linear_addr, 39, 47,
	PG_TRVRSE_BASIC_VA_ADDR);
	return (uint64_t *) pml4e_virtual_addr;
}

uint64_t * virtual_addr_pdirptre(uint64_t linear_addr) {
// copy the pml offset and pdirptr offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
//as table and page offsets respectively.
	uint64_t temp = extract_bits(linear_addr, 39, 47,
	PG_TRVRSE_BASIC_VA_ADDR, 12, 20);
	uint64_t pdir_ptre_virtual_addr = copy_as_pageoffset(linear_addr, 30, 38,
			temp);
	return (uint64_t *) pdir_ptre_virtual_addr;
}

uint64_t * virtual_addr_pdire(uint64_t linear_addr) {
// copy pml, pdirptr and dir offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
// as dir, table and page offsets respectively
	uint64_t temp = extract_bits(linear_addr, 39, 47, PG_TRVRSE_BASIC_VA_ADDR,
			21, 29);
	temp = extract_bits(linear_addr, 30, 38, temp, 12, 20);
	uint64_t pdire_virtual_addr = copy_as_pageoffset(linear_addr, 21, 29, temp);
	return (uint64_t *) pdire_virtual_addr;
}

uint64_t * virtual_addr_pte(uint64_t linear_addr) {
// copy pml, pdirptr, dir and table offset from linear addr to PG_TRVRSE_BASIC_VA_ADDR
// as dirptr, dir, table and page offsets respectively
	uint64_t temp = extract_bits(linear_addr, 39, 47, PG_TRVRSE_BASIC_VA_ADDR,
			30, 38);
	temp = extract_bits(linear_addr, 30, 38, temp, 21, 29);
	temp = extract_bits(linear_addr, 21, 29, temp, 12, 20);
	uint64_t pte_virtual_addr = copy_as_pageoffset(linear_addr, 12, 20, temp);
	return (uint64_t *) pte_virtual_addr;
}

//todo: verify this is working and use cleanup in virtual addr space
void cleanup_page(uint64_t * page_table_entity_entry) {
	uint64_t page_table_entity_base = extract_bits(
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

static inline void invalidate_tlb(uint64_t* m) {
	__asm__ __volatile__ ( "invlpg (%0)" : : "b"(m) : "memory" );
}

void invalidate_addresses_with_page(uint64_t * virtual_addr) {
	uint64_t temp = extract_bits((uint64_t) virtual_addr, 12, 63, ULONG_ZERO,
			12, 63);
// there are 4096 linear addresses whose page = that of this virtual addr
// and all of them need to be flushed
	for (uint64_t i = 0; i < PAGE_SIZE; i++) {
		uint64_t addr = extract_bits(i, 0, 11, temp, 0, 11);
//		printf("clearing %p ", addr);
		invalidate_tlb((uint64_t *) addr);
	}
}

void zero_out_using_virtual_addr(uint64_t linear_addr) {
	uint64_t* addr_base = (uint64_t*) (linear_addr & (~0xfff));
	for (int i = 0; i < NUM_UNIT64_IN_PAGE; i++) {
		*(addr_base + i) = 0;
	}
}
//**** DO-NOT CALL THIS DIRECTLY. USE OTHER SETUP_PAGE_TABLE.. FUNCS ***
void setup_page_tables_after_cr3_update(uint64_t linear_addr,
		uint64_t physical_addr, int p, int rw, int us) {
// derive paging entities from linear address and update their target
//	printf("\nlinear_addr is %p, physical:%p\n", linear_addr, physical_addr);
	uint64_t * pml4e_virtual_addr = virtual_addr_pml4e(linear_addr);
	if (is_entry_not_created((uint64_t *) pml4e_virtual_addr)) {
		uint64_t* pdir_ptr = get_free_zeroed_frame();
		*pml4e_virtual_addr = get_pml4_entry((uint64_t) pdir_ptr, p, rw, us);
//		uint64_t* virtualAddrPdirptre = virtual_addr_pdirptre(linear_addr);
//		cleanup_page(virtualAddrPdirptre);
	}
//	printf("\npml4e_virtual_addr:%p, value:%p\n", pml4e_virtual_addr,
//			*pml4e_virtual_addr);

	uint64_t * pdir_ptre_virtual_addr = virtual_addr_pdirptre(linear_addr);
	if (is_entry_not_created((uint64_t *) pdir_ptre_virtual_addr)) {
		uint64_t* pdir = get_free_zeroed_frame();
		*pdir_ptre_virtual_addr = get_pdpt_entry((uint64_t) pdir, p, rw, us);
	}
//	printf("\npdir_ptre_virtual_addr:%p, value:%p\n", pdir_ptre_virtual_addr,
//			*pdir_ptre_virtual_addr);

	uint64_t * pdire_virtual_addr = virtual_addr_pdire(linear_addr);
	if (is_entry_not_created((uint64_t *) pdire_virtual_addr)) {
		uint64_t* ptable = get_free_zeroed_frame();
		*pdire_virtual_addr = get_pd_entry((uint64_t) ptable, p, rw, us);
	}
//	printf("\npdire_virtual_addr:%p, value:%p\n", pdire_virtual_addr,
//			*pdire_virtual_addr);

	uint64_t * pte_virtual_addr = virtual_addr_pte(linear_addr);
	int present_before = !(is_entry_not_created((uint64_t *) pte_virtual_addr));
	*pte_virtual_addr = get_ptable_entry(physical_addr, p, rw, us);
	if (present_before) {
		// invalidate TLB entry
//		printf("!!!invalidating TLB!!!");
		invalidate_addresses_with_page((uint64_t *) linear_addr);
	}
//	printf("\npte_virtual_addr:%p, value:%p\n", pte_virtual_addr,
//			*pte_virtual_addr);

}

void setup_process_page_tables(uint64_t linear_addr, uint64_t physical_addr) {
	setup_page_tables_after_cr3_update(linear_addr, physical_addr, 1, 1, 1);
	zero_out_using_virtual_addr(linear_addr);
}

// will be needed for COW where processes share target physical pages
void setup_process_page_tables_without_zeroing(uint64_t linear_addr,
		uint64_t physical_addr) {
	setup_page_tables_after_cr3_update(linear_addr, physical_addr, 1, 1, 1);
}

void setup_kernel_page_tables(uint64_t linear_addr, uint64_t physical_addr) {
	setup_page_tables_after_cr3_update(linear_addr, physical_addr, 1, 1, 0);
	zero_out_using_virtual_addr(linear_addr);
}

// IMPORTANT - do not use virtual addresses corresponding to 510 PML offset
void map_page_tables_adress(uint64_t **pml_base_dbl_ptr, int p, int rw, int us) {
	uint64_t *pml_base_ptr = *pml_base_dbl_ptr;
	uint64_t * temp = pml_base_ptr + 510;
	*temp = get_pml4_entry((uint64_t) pml_base_ptr, p, rw, us);
//	printf("*pml_base_ptr is %p\n", *pml_base_ptr);
}

void map_other_addresses(uint64_t **pml_base_dbl_ptr, int p, int rw, int us,
		int len_freelist) {
//mapping the free list
	uint64_t virtual_free_list = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list;
	uint64_t free_list_temp = (uint64_t) free_list;
	for (int i = 0; i < len_freelist; i++) {	//free pages is 3 pages long
		setup_page_tables(pml_base_dbl_ptr, virtual_free_list, free_list_temp,
				p, rw, us);
		virtual_free_list += 4096;
		free_list_temp += 4096;
	}

	free_list = (page_t *) (VIRTUAL_PHYSFREE_OFFSET + (uint64_t) free_list);//change the free_list address from physical to virtual
}
void map_tarfs_addresses(uint64_t **pml_base_dbl_ptr, int p, int rw, int us) {
	uint64_t virtual_addr = 0xffffffff8020c000;
	uint64_t physical_addr = 0x20c000;
	for (int i = 0; i < 37; i++) {
		setup_page_tables(pml_base_dbl_ptr, virtual_addr, physical_addr, p, rw,
				us);
		virtual_addr = virtual_addr + 0x1000;
		physical_addr = physical_addr + 0x1000;
	}
}
void map_linear_addresses(uint64_t **pml_base_dbl_ptr, void* physbase,
		void* physfree, int p, int rw, int us, int len_freelist) {
	map_kernel_address(pml_base_dbl_ptr, physbase, physfree, p, rw, us);

	map_video_address(pml_base_dbl_ptr, p, rw, us);
	map_other_addresses(pml_base_dbl_ptr, p, rw, us, len_freelist);	//this function maps other random addresses we might randomly map after physfree. Example: freelist
	map_page_tables_adress(pml_base_dbl_ptr, p, rw, us);
//	map_tarfs_addresses(pml_base_dbl_ptr, p, rw, us);
}

void update_cr3(uint64_t * pml_base_ptr) {
	uint64_t pml_base_40bits = update40bit_addr(ULONG_ZERO,
			(uint64_t) pml_base_ptr);
//	printf("pre cr3 update");
	__asm__ __volatile(
			"cli\n\t"
			"movq %0,%%cr3"
			:
			:"a"(pml_base_40bits)
	);
//	printf("post cr3 update");
}
void manage_memory_test_suite() {
	uint64_t *ret = get_free_zeroed_frame();
//	printf("\nans1: %p", ret);
	uint64_t virtual_test_addr = VIRTUAL_PHYSFREE_OFFSET + (uint64_t) ret;
	uint64_t test_addr = (uint64_t) ret;
	setup_page_tables_after_cr3_update(virtual_test_addr, test_addr, 1, 1, 0);
	*((uint64_t *) virtual_test_addr) = 0xb00b;
	printf("\nmem access after ptable-setup %x\n",
			*((uint64_t *) virtual_test_addr));
	setup_page_tables_after_cr3_update(virtual_test_addr, test_addr + 5000, 1,
			1, 0);
	printf("diff mem access after ptable-setup %x\n",
			*((uint64_t *) virtual_test_addr));
	setup_page_tables_after_cr3_update(0x1234455355, test_addr, 1, 1, 0);
	printf("another mem access after ptable-setup %x\n",
			*((uint64_t *) 0x1234455355));

//	struct page_t *test = (struct page_t *)virtual_test_addr;
//	test[0].is_free = 0;
//	test[0].ref_count = 2;
//	test[0].virtual_address = NULL;
//	printf("YAYYYY:  %d", test[0].ref_count);
//	ret = get_free_zeroed_frames(1);
//	printf("\nans2: %p", ret);
//	ret = get_free_zeroed_frames(2);
//	printf("\nans3: %p", ret);
//	ret = get_free_zeroed_frames(3);
//	printf("\nans3: %p", ret);
//	ret = get_free_zeroed_frames(4);
//	printf("\nans3: %p", ret);
//
//	ret = get_free_zeroed_frames(5);
//	printf("\nans3: %p", ret);
//	ret = get_free_zeroed_frames(6);
//	printf("\nans3: %p", ret);
//	uint64_t *ret2 = get_free_zeroed_frames(2);
//	printf("\n%x", ret2);
}

// Kernel paging
void do_paging(void* physbase, void* physfree, int p, int rw, int us,
		int len_freelist) {
	map_linear_addresses(&kernel_pml_base_ptr, physbase, physfree, p, rw, us,
			len_freelist);
	update_cr3(kernel_pml_base_ptr);
}
uint64_t current_virtual_cr3 = 0xffffffffa0000000;
void *get_virtual_location_cr3(int order) {
	void *return_loc = (void *) current_virtual_cr3;
	current_virtual_cr3 += (0x1000) * (1 << order); //shifting by those many pages.
	return return_loc;
}

uint64_t* get_pml4_base_addrs_for_process(uint64_t* process_pml_base_virtual) {
// copy the kernel's pml4 base frame into
// a new frame and return the physical address
// so that the task_struct can store the same
	uint64_t* process_pml_base_physical = get_free_zeroed_frame(0);
	uint64_t* virtual_addr = (uint64_t*) get_virtual_location_cr3(0);
//	printf("va : %p\n", virtual_addr);
// we are copying the kernel's address space to the process'
// so use setup_kernel_page_tables so that the permissions are
// set appropriately and not setup_processl_page_tables.
// When process' page address space needs to be updated
// use setup_process_page_tables
	setup_kernel_page_tables((uint64_t) virtual_addr,
			(uint64_t) process_pml_base_physical);
	uint64_t *ptr = (uint64_t *) KERNEL_PML4_BASE_VIRTUAL;
// kernel addresses only lie in pml 511, so just copy that pml4e
	*(virtual_addr + 511) = *(ptr + 511);
// self referencing of the page table
	uint64_t* temp = virtual_addr + 510;
	*temp = get_pml4_entry((uint64_t) process_pml_base_physical, 1, 1, 0);

	*process_pml_base_virtual = (uint64_t) virtual_addr;
	return process_pml_base_physical;
}

uint64_t * get_physical_pml4_base_for_process() {
	uint64_t process_pml_base_virtual = 0;
	uint64_t* process_pml_base_physical = get_pml4_base_addrs_for_process(
			&process_pml_base_virtual);
	return process_pml_base_physical;
}

void get_both_pml4_base_addrs_for_process(uint64_t* vaddr, uint64_t* phys_addr) {
	*phys_addr = (uint64_t) get_pml4_base_addrs_for_process(vaddr);
}

int is_linear_addr_mapped(uint64_t linear_addr) {
	uint64_t * pml4e_virtual_addr = virtual_addr_pml4e(linear_addr);
	if (!is_entry_not_created((uint64_t *) pml4e_virtual_addr)) {
		uint64_t * pdir_ptre_virtual_addr = virtual_addr_pdirptre(linear_addr);

		if (!is_entry_not_created((uint64_t *) pdir_ptre_virtual_addr)) {
			uint64_t * pdire_virtual_addr = virtual_addr_pdire(linear_addr);

			if (!is_entry_not_created((uint64_t *) pdire_virtual_addr)) {
				uint64_t * pte_virtual_addr = virtual_addr_pte(linear_addr);
				int is_pte_present = !(is_entry_not_created(
						(uint64_t *) pte_virtual_addr));
				return is_pte_present;
			}
		}
	}
	return 0;
}

void manage_memory(void* physbase, void* physfree, uint32_t* modulep) {
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
	//printf("PHYSBASE:%p PHYSFREE: %p\n", physbase, physfree);
//can write a better check below. Your wish
	uint64_t number_pages_free_list = (MAX_NUMBER_PAGES * sizeof(struct page_t))
			/ PAGE_SIZE + 1; //potentially wasting a page if it exactly page sizes here but thats fine and rare
//	printf(" NUMBER OF PAGES TAKEN by FREE_PAGES %d\n", number_pages_free_list);
	for (int i = 0; i < number_pages_free_list; i++) {
		mark_frame_used((uint64_t) free_list_location + i * (0x1000));
	}
	mark_frame_used(0xb8000);
//	blank_space_baby(free_list);
	do_paging(physbase, physfree, 1, 1, 0, number_pages_free_list);

//	uint64_t *temp = get_physical_pml4_base_for_process();
//	update_cr3(temp);
//	printf("process_pml4: %p ", temp);
//	uint64_t * q = (uint64_t *) 0x400038;
//	setup_process_page_tables((uint64_t)q, (uint64_t)get_free_zeroed_frame());
//	*q = 0xf00d;
//	printf("new var: %p ", *q);
//	printf("fasaf");

//	manage_memory_test_suite();
//	printf("\npresence:::%d ", is_linear_addr_mapped(0xFFFFFF7FBFDFEFF0));
}

void process_stuff() {
	printf("testing new process' pml set and variable setting\n");
	uint64_t* temp = get_physical_pml4_base_for_process();
	update_cr3(temp);
	printf("process_pml4: %p ", temp);
	uint64_t* q = (uint64_t *) 0x400038;
	setup_process_page_tables((uint64_t) q, (uint64_t) get_free_zeroed_frame());

	printf("ismapped %d \n", is_linear_addr_mapped((uint64_t) q));
	*q = 0xf00d;
	printf("new var: %p \n", *q);
}

int do_pmls_clash(uint64_t addr1, uint64_t addr2) {
	struct paging_entities pe1;
	struct paging_entities pe2;
	get_paging_entity_indexes(&pe1, addr1);
	get_paging_entity_indexes(&pe2, addr2);
	return pe1.pml_index == pe2.pml_index;
}

uint64_t phys_addr_of_frame(uint64_t virtual_addr) {
	uint64_t * target_pte = virtual_addr_pte(virtual_addr);
	return (uint64_t) next_entity_base(target_pte);
}

uint64_t vaddr_of_ptable(uint64_t virtual_addr) {
	uint64_t * target_pte = virtual_addr_pte(virtual_addr);
	return ((uint64_t) target_pte) & (~0xfff);
}

uint64_t vaddr_of_pdir(uint64_t virtual_addr) {
	uint64_t * target_pdire = virtual_addr_pdire(virtual_addr);
	return ((uint64_t) target_pdire) & (~0xfff);
}

uint64_t vaddr_of_pdir_ptr(uint64_t virtual_addr) {
	uint64_t * target_pdir_ptre = virtual_addr_pdirptre(virtual_addr);
	return ((uint64_t) target_pdir_ptre) & (~0xfff);
}


