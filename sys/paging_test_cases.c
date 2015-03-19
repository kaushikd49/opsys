#include<sys/paging.h>
#include<sys/defs.h>
#include <sys/sbunix.h>

uint64_t * setup_page_tables(uint64_t linear_addr, uint64_t physical_addr);
int page_lookup(uint64_t linear_addr, uint64_t* deepest_entity,
		uint64_t* deepest_entity_base);
void manage_memory(void* physbase, void* physfree, uint32_t* modulep);

void print_result_check(uint64_t addr_to_lookup, int expected_res) {

	uint64_t* deepest_entity = NULL;
	uint64_t* deepest_entity_base = NULL;

	int res = page_lookup(addr_to_lookup, deepest_entity, deepest_entity_base);

	if (res != expected_res) {
		printf("!!!!!!!!!! Assert failure: expected != res %d!=%d !!!!!!!!!!!!",
				expected_res, res);
	}
	printf(
			"lookup result should be %d, is:%d, deepest-entity:%p, its value:%p, deep base:%p\n",
			expected_res, res, *deepest_entity, *(uint64_t*) (*deepest_entity),
			*deepest_entity_base);
}

void testLookup() {
	uint64_t a = 0xffffffff80200000; //ofst:0,  tabl:0,  dir:1,  dir_ptr:510,  pml:511
	uint64_t e = 0xffffffff80210000; //ofst:0,  tabl:16,  dir:1,  dir_ptr:510,  pml:511
	uint64_t d = 0xffffffff81210000; //ofst:0,  tabl:16,  dir:9,  dir_ptr:510,  pml:511
	uint64_t c = 0xffffffff21410000; //ofst:0,  tabl:16,  dir:266,  dir_ptr:508,  pml:511
	uint64_t b = 0xfffffaba21410000; //ofst:0,  tabl:16,  dir:266,  dir_ptr:232,  pml:501

	// duplicate setup for same address
	setup_page_tables(a, 0x12345);
	setup_page_tables(a, 0x12345);
	print_result_check(a, 0);
	// partial page table entities presence check
	print_result_check(e, 4);
	print_result_check(d, 3);
	print_result_check(c, 2);
	print_result_check(b, 1);

	// previously added page table entries should not be
	// affected by adding totally non-intersecting page table entities
	setup_page_tables(b, 0x12345);
	print_result_check(b, 0);
	print_result_check(a, 0);

	// previously added page table entries should not be
	// affected by adding intersecting page table entities
	setup_page_tables(e, 0x12345);
	print_result_check(b, 0);
	print_result_check(a, 0);
	// d should be unaffected, hence 3
	print_result_check(d, 3);

}

void pagingTests(void* physbase, void* physfree, uint32_t* modulep) {
	manage_memory(physbase, physfree, modulep);
	testLookup();
}
