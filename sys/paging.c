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

//uint64_t get_free_pages();
uint64_t * pml_base_ptr = NULL;

// todo: dummy below
uint64_t * get_free_pages() {
	return (uint64_t *) 1;
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

// todo: dummy below
uint64_t get_pml4_entry() {
	uint64_t entry = set_bit(ULONG_ZERO, 0, 1); // Present
	entry = set_bit(ULONG_ZERO, 1, 1); // Read/Write
	// 51:12 fill 40 bit address
	// 0, 1,
	return entry;
}

uint64_t get_pdpt_entry() {
	return 1;
}

uint64_t get_pd_entry() {
	return 1;
}

uint64_t get_ptable_entry() {
	return 1;
}
// todo: dummy above

void set_paging(int pml4_num, int pdir_ptr_num, int dir_num, int table_num) {
	if (pml4_num > PML_SIZE || pdir_ptr_num > PDPTR_SIZE || dir_num > PD_SIZE
			|| table_num > PT_SIZE) {
//		printf("dirnum or tablenum exeecind limits. exiting");
		return;// no exit yet
	}

	if (pml_base_ptr == NULL) {
		pml_base_ptr = get_free_pages();
	}

	uint64_t *pdir_ptr = get_free_pages();
	uint64_t *pdir = get_free_pages();
	uint64_t *ptable = get_free_pages();

	*(pml_base_ptr + pml4_num - 1) = get_pml4_entry();
	*(pdir_ptr + pdir_ptr_num - 1) = get_pdpt_entry();
	*(pdir + dir_num - 1) = get_pd_entry();
	*(ptable + table_num - 1) = get_ptable_entry();

}
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

void manage_memory(void* physfree, uint32_t* modulep) {
	// kernel starts here
	printf("\ncreating free list");
	uint64_t temp = (uint64_t) physfree;
	printf("\nlocation of physfree+temp: %x", temp);
	uint64_t free_list_location = (uint64_t) ((((uint64_t) physfree)
			& (~(PAGE_SIZE - 1))) + (PAGE_SIZE));
	printf("\nlocation of free list: %x", free_list_location);
	char* free_list = (char*) (free_list_location);
	create_free_list(modulep, free_list);
	uint64_t ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	return_page(ret, free_list);
	ret = get_free_page(free_list);
	printf("\nans: %x", ret);
	return_page(0x1000, free_list);
	printf("\ndone");
}

