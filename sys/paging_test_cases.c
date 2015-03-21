#include<sys/paging.h>
#include<sys/defs.h>
#include <sys/sbunix.h>
# define NULL1 ((void*)1)
# define NULL2 ((void*)2)

uint64_t * setup_page_tables(uint64_t linear_addr, uint64_t physical_addr);
int page_lookup(uint64_t linear_addr, uint64_t* deepest_entity,
		uint64_t* deepest_entity_base);
void manage_memory(void* physbase, void* physfree, uint32_t* modulep);

void print_result_check(uint64_t addr_to_lookup, int expected_res) {

	uint64_t abc = 1;
	uint64_t def = 2;
	uint64_t * deepest_entity = &abc;
	uint64_t * deepest_entity_base = &def;

	int res = page_lookup(addr_to_lookup, deepest_entity, deepest_entity_base);

	if (res != expected_res) {
		printf("!!!!!!!!!! Assert failure: expected != res %d!=%d for %p!!!!!!!!!!!!\n",
				expected_res, res, addr_to_lookup);
	}

//	printf(
//			"lookup result should be %d, is:%d, deepest-entity:%p, its value:%p, deep base:%p\n",
//			expected_res, res, *deepest_entity, *(uint64_t*) (*deepest_entity),
//			*deepest_entity_base);
}

void testLookup() {
	uint64_t a = 0xffffffff80200000; //ofst:0,  tabl:0,  dir:1,  dir_ptr:510,  pml:511
	uint64_t e = 0xffffffff80210000; //ofst:0,  tabl:16,  dir:1,  dir_ptr:510,  pml:511
	uint64_t d = 0xffffffff81210000; //ofst:0,  tabl:16,  dir:9,  dir_ptr:510,  pml:511
	uint64_t c = 0xffffffff21410000; //ofst:0,  tabl:16,  dir:266,  dir_ptr:508,  pml:511
	uint64_t b = 0xfffffaba21410000; //ofst:0,  tabl:16,  dir:266,  dir_ptr:232,  pml:501
	int linearAddr = 0x226000;

	// duplicate setup for same address
	setup_page_tables(a, linearAddr);
	setup_page_tables(a, linearAddr);
	print_result_check(a, 0);
	// partial page table entities presence check
	print_result_check(e, 4);
	print_result_check(d, 3);
	print_result_check(c, 2);
	print_result_check(b, 1);

	// previously added page table entries should not be
	// affected by adding totally non-intersecting page table entities
	setup_page_tables(b, linearAddr);
	print_result_check(b, 0);
	print_result_check(a, 0);

	// previously added page table entries should not be
	// affected by adding intersecting page table entities
	setup_page_tables(e, linearAddr);
	print_result_check(b, 0);
	print_result_check(a, 0);
	// d should be unaffected, hence 3
	print_result_check(d, 3);

	setup_page_tables(d,linearAddr);
	print_result_check(d, 0);
}

void pagingTests(void* physbase, void* physfree, uint32_t* modulep) {
	manage_memory(physbase, physfree, modulep);
	testLookup();
}
