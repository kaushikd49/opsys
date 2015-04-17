#ifndef FREELIST_H
#define FREELIST_H
#include<sys/defs.h>
#include<sys/pagingglobals.h>

int check_physical_frame(uint64_t current_addr, physical_map_node *test, uint64_t num_physical_regions, uint64_t physbase, uint64_t physfree);
//marks the address that that you provide as argument. If the address is not physically aligned then it does nothing.
void mark_frame_used(uint64_t address);
/*this function has not been completed yet, we will get to it as and when we need it.*/
void create_free_list_test(uint32_t* modulep, page_t *free_list, void *physbase, void *physfree);
uint64_t get_free_page(page_t *free_list);
//legacy helper functions not needed
int check_boolarray_index(char *array, uint64_t pos, uint64_t limit);
//the start and end should be a valid offset. no check done
int check_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
void clear_boolarray_index(char *array, uint64_t pos, uint64_t limit);
void clear_boolarray_range(char *array, uint64_t start, uint64_t end, uint64_t limit);
//legacy boolarray functions end here
//be careful if you use this function for other stuff
//this function checks 2 things. First, it checks if the range itself is contiguous and also, that the memory addresses it references are contiguous.
int check_array_range(page_t *free_list, uint64_t start, uint64_t end);
void clear_array_range(page_t *free_list, uint64_t start, uint64_t end);
uint64_t get_free_pages(page_t *free_list,int order);
void return_page(uint64_t page, page_t *free_list);
//no checks done, programmer care required.
void return_pages(uint64_t page, page_t *free_list, int order);
uint64_t * get_free_frame();
uint64_t * get_free_frames(int order);
int get_ref_count(uint64_t physical_addr_page);

void decrease_ref_count(uint64_t physical_addr_page);

void increase_ref_count(uint64_t physical_addr_page);
#endif
