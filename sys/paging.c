#include <sys/defs.h>
#include <sys/sbunix.h>
#define PD_SIZE 512
#define PT_SIZE 512
#define PML_SIZE 512
#define PDPTR_SIZE 512
#define ULONG_ONE ((unsigned long)1) // without this left shift  more than 31 will not work
#define ULONG_ZERO ((unsigned long)0)

#define MAX_NUMBER_PAGES (1<<18)
#define PAGE_SIZE (1<<12)
#define PAGE_ALIGN (1<<12)
#define CHAR_SIZE (1<<3)
#define MAX_FREELIST_LENGTH (MAX_NUMBER_PAGES/CHAR_SIZE)

uint64_t * pml_base_ptr = NULL; // equivalent to CR3

char* free_list = NULL;
uint64_t free_list_location = ULONG_ZERO;

// todo: duplicated smap definition from main.
struct smap_t {
	uint64_t base, length;
	uint32_t type;
}__attribute__((packed));
void create_free_list(uint32_t* modulep, char *free_list) {
	uint64_t i;
	uint64_t current_index;
	uint64_t current_bit;
	for (i = 0; i < MAX_FREELIST_LENGTH; i++) //char
		free_list[i] = 0;
	printf("\nfree list size%d", i);
	struct smap_t* smap;
	for (smap = (struct smap_t*) (modulep + 2);
			smap < (struct smap_t*) ((char*) modulep + modulep[1] + 2 * 4);
			++smap) {
		if (smap->type == 1 /* memory */&& smap->length != 0) {
			uint64_t start = ((smap->base) + PAGE_ALIGN - 1)
					& ~(PAGE_ALIGN - 1);
			current_index = (start >> 3); //right shift 6 for the index
			current_bit = (start & 0xff);
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
uint64_t get_free_page(char *free_list) {
	int k, i = 0, check = 0; //todo: could optimize to search from previous page given
	for (; i < MAX_FREELIST_LENGTH; i++) {
		if ((uint64_t) (free_list[i]) > 0) {
			k = 0;
			while (k < 8 && (uint64_t) (((1 << k) & free_list[i])) == 0) {
				k++;
			}
			if (k < 8) {
				check = 1;
				break;
			}
		}
	}
	if (check == 1) {
		uint64_t return_val = (i * 8 + k) << 12;
		free_list[i] ^= 1 << k;
		return return_val;
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

// todo: dummy below
uint64_t * get_free_frame() {
	uint64_t freePage = get_free_page(free_list);
	printf("returning freepage:%p\n", freePage);
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

uint64_t get40bit_addr(uint64_t entry, uint64_t extract_from) {
	// 51:12 fill 40 bit address
	entry = extract_bits(extract_from, 12, 51, entry, 12, 51);
	return entry;
}

uint64_t get_pml4_entry(uint64_t pdir_ptr) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return get40bit_addr(entry, pdir_ptr);
}

uint64_t get_pdpt_entry(uint64_t pdir) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return get40bit_addr(entry, pdir);
}

uint64_t get_pd_entry(uint64_t ptable) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return get40bit_addr(entry, ptable);
}

uint64_t get_ptable_entry(uint64_t physical_addr) {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(entry, 1, 1); // Read/Write
	return get40bit_addr(entry, physical_addr);
}
// todo: dummy above

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

	printf("pdirptr %p\n", pdir_ptr);
	printf("pdir %p\n", pdir);
	printf("ptable %p\n", ptable);

	printf("pml_base_ptr + pe.pml_index  %p\n", pml_base_ptr + pe.pml_index);
	printf("pdir_ptr + pe.pdir_ptr_index  %p\n", pdir_ptr + pe.pdir_ptr_index);
	printf("pdir + pe.dir_index %p\n", pdir + pe.dir_index);
	printf("ptable + pe.table_index %p\n", ptable + pe.table_index);

	*(pml_base_ptr + pe.pml_index) = get_pml4_entry((uint64_t) pdir_ptr);
	*(pdir_ptr + pe.pdir_ptr_index) = get_pdpt_entry((uint64_t) pdir);
	*(pdir + pe.dir_index) = get_pd_entry((uint64_t) ptable);
	*(ptable + pe.table_index) = get_ptable_entry(physical_addr);

	printf("stored pml_base_ptr + pe.pml_index  %p\n",
			*(pml_base_ptr + pe.pml_index));
	printf("stored pdir_ptr + pe.pdir_ptr_index  %p\n",
			*(pdir_ptr + pe.pdir_ptr_index));
	printf("stored pdir + pe.dir_index %p\n", *(pdir + pe.dir_index));
	printf("stored ptable + pe.table_index %p\n", *(ptable + pe.table_index));

	return ptable + pe.table_index;
}

int is_not_allocated(uint64_t* entry) {
	return *entry == 0; //todo: check this logic
}

uint64_t* next_entity_entry(uint64_t* entity_entry, int offset) {
	uint64_t *next_entity_base = (uint64_t *) get40bit_addr(ULONG_ZERO,
			*entity_entry);
	uint64_t* next_entry = next_entity_base + offset;
	return next_entry;
}

// Entity - pml, pdir_ptr, pdir or ptable
// Return whether pml, pdir_ptr, pdir or ptable is not present
// Status for each would be indicated by 1,2,3,4 respectively
// If all are present, then return 0.
// Also set the deepest entity present to deepest_entity
int page_lookup(uint64_t linear_addr, uint64_t* deepest_entity) {
	struct paging_entities pe;
	get_paging_entity_indexes(&pe, linear_addr);

	uint64_t *pml = pml_base_ptr + pe.pml_index;
	printf("pml lookup %p\n", pml);

	if (is_not_allocated(pml)) {
		return 1;
	} else {
		*deepest_entity = (uint64_t) pml;
		uint64_t* pdir_ptr = next_entity_entry(pml, pe.pdir_ptr_index);
		printf("pdir_ptr lookup %p\n", pdir_ptr);

		if (is_not_allocated(pdir_ptr)) {
			return 2;
		} else {
			*deepest_entity = (uint64_t) pdir_ptr;
			uint64_t* pdir = next_entity_entry(pdir_ptr, pe.dir_index);
			printf("pdir lookup %p\n", pdir);

			if (is_not_allocated(pdir)) {
				return 3;
			} else {
				*deepest_entity = (uint64_t) pdir;
				uint64_t* ptable = next_entity_entry(pdir, pe.table_index);
				printf("ptable lookup %p\n", ptable);
				if (is_not_allocated(ptable)) {
					return 4;
				} else {
					*deepest_entity = (uint64_t) ptable;
					return 0;
				}
			}
		}
	}
}

void manage_memory(void* physbase, void* physfree, uint32_t* modulep) {
	// kernel starts here
	printf("\ncreating free list");
	uint64_t temp = (uint64_t) physfree;
	printf("\nlocation of physfree+temp: %x", temp);

	if (free_list_location == ULONG_ZERO) {
		free_list_location = (uint64_t) ((((uint64_t) physfree)
				& (~(PAGE_SIZE - 1))) + (PAGE_SIZE));
		printf("\nlocation of free list: %x", free_list_location);
	}
	if (free_list == NULL) {
		free_list = (char*) (free_list_location);
	}

	create_free_list(modulep, free_list);

	uint64_t ret = get_free_page(free_list);
	printf("\nans: %p", ret);

	ret = get_free_page(free_list);
	printf("\nans: %p", ret);
	return_page(ret, free_list);

	ret = get_free_page(free_list);
	printf("\nans: %p", ret);
	return_page(0x1000, free_list);

	uint64_t* deepest_entity = NULL;
	set_paging(0xffffffff80200000, (uint64_t) physbase);
	int res = page_lookup(0xffffffff80200000, deepest_entity);

	printf("lookup res is:%d, deepest-entity%p", res, *deepest_entity);
}

