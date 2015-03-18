#include <sys/defs.h>

#define PD_SIZE 512
#define PT_SIZE 512
#define PML_SIZE 512
#define PDPTR_SIZE 512
#define ONE ((unsigned long)1) // without this left shift  more than 31 will not work

//#define NULL (void *)

//uint64_t get_free_pages();
uint64_t * pml_base_ptr = NULL;

// todo: dummy below
uint64_t * get_free_pages() {
	return (uint64_t *) 1;
}

uint64_t set_bit(uint64_t ele, int bit_num, int bit_val) {
	uint64_t bit_in_uint64 = ONE << bit_num;
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
		uint64_t bit_in_uint64 = ONE << i;
		unsigned int ith_bit = (from & bit_in_uint64) >> i;
		to = set_bit(to, j, ith_bit);
	}
	return to;
}

// todo: dummy below
uint64_t get_pml4_entry() {
	return 1;
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

