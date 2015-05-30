#ifndef KMALLOC_H
#define KMALLOC_H
void init_caches();
void init_cache_list();
void *get_virtual_location(int order);
void prepare_page(int order, void *address);
void *get_mem_alloc(int order);
void *get_pages_directly(int order);
void *allocate_in_cache(int order);
void dealloc_in_cache(int order, void *ptr);
void *kmalloc(uint64_t size);
void kfree(void *addr);
int is_kernel_addr(uint64_t addr);
uint64_t next_power_two(uint64_t v);
uint64_t find_order(uint64_t size);
#endif
