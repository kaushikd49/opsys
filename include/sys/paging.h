#ifndef PAGING_H
#define PAGING_H
#include <sys/defs.h>
uint64_t set_bit(uint64_t ele, int bit_num, int bit_val);
uint64_t extract_bits(uint64_t from, int fstart_bit, int fend_bit, uint64_t to,
		int tstart_bit, int tend_bit);
void manage_memory(void* physbase, void* physfree, uint32_t* modulep);
void pagingTests(void* physbase, void* physfree, uint32_t* modulep);
void setup_process_page_tables(uint64_t linear_addr, uint64_t physical_addr);
void setup_kernel_page_tables(uint64_t linear_addr, uint64_t physical_addr);
int is_linear_addr_mapped(uint64_t linear_addr);
uint64_t * get_physical_pml4_base_for_process();
void setup_process_page_tables(uint64_t linear_addr, uint64_t physical_addr);
void process_stuff();
void update_cr3(uint64_t * pml_base_ptr);
int do_pmls_clash(uint64_t addr1, uint64_t addr2);
int get_bit(uint64_t num, int bit_num);
uint64_t * virtual_addr_pte(uint64_t linear_addr);
void invalidate_addresses_with_page(uint64_t * virtual_addr);

void setup_page_table_from_outside(uint64_t linear_addr, uint64_t physical_addr,
		int p, int rw, int us, uint64_t** pml_base_dbl_ptr,
		uint64_t* (*addr_map_func)(uint64_t*, void *), void * pv_map);

uint64_t* next_entity_base(uint64_t* entity_entry);

void get_both_pml4_base_addrs_for_process(uint64_t* vaddr, uint64_t* phys_addr);

void setup_page_tables_after_cr3_update(uint64_t linear_addr,
		uint64_t physical_addr, int p, int rw, int us);

uint64_t phys_addr_of_frame(uint64_t virtual_addr);

uint64_t * virtual_addr_pml4e(uint64_t linear_addr);
uint64_t * virtual_addr_pdirptre(uint64_t linear_addr);
uint64_t * virtual_addr_pdire(uint64_t linear_addr);
uint64_t * virtual_addr_pte(uint64_t linear_addr);
#endif
