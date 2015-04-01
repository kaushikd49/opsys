#ifndef PAGING_PROCESS_H
#define PAGING_PROCESS_H
#include <sys/defs.h>
uint64_t set_bit_process(uint64_t ele, int bit_num, int bit_val);
uint64_t extract_bits_process(uint64_t from, int fstart_bit, int fend_bit, uint64_t to, int tstart_bit, int tend_bit) ;
void manage_memory_process(void* physbase, void* physfree, uint32_t* modulep);
void pagingTests_process(void* physbase, void* physfree, uint32_t* modulep) ;
void setup_page_tables_after_cr3_update_process(uint64_t linear_addr,uint64_t physical_addr);
void do_paging_process(void* physbase, void* physfree, uint64_t *pml_base_ptr_process);
#endif
