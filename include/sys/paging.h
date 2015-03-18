#include <sys/defs.h>
uint64_t set_bit(uint64_t ele, int bit_num, int bit_val);
uint64_t extract_bits(uint64_t from, int fstart_bit, int fend_bit, uint64_t to, int tstart_bit, int tend_bit) ;
void manage_memory(void* physfree, uint32_t* modulep);
