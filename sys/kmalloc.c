
#include<sys/defs.h>
#include<sys/sbunix.h>
#include<sys/paging.h>
#include<sys/freelist.h>
#include<sys/pagingglobals.h>
#define ALIGNMENT 32
cache_t *cache_list = NULL;
cache_init cache_info = { NULL, NULL };
uint64_t BASE_MEMORY_MANAGER = 0xffffffffb0000000; //********** fill this up
// todo: change this base addr so that kernel addr space is well utilized

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
void init_cache_list(cache_t *cache_list) {
	cache_list[0].order = 5;
	//cache_list[0].head = NULL;
	cache_list[0].free = NULL;
	cache_list[1].order = 6;
	//cache_list[1].head = NULL;
	cache_list[1].free = NULL;
	cache_list[2].order = 7;
	//cache_list[2].head = NULL;
	cache_list[2].free = NULL;
	cache_list[3].order = 8;
	//cache_list[3].head = NULL;
	cache_list[3].free = NULL;
	cache_list[4].order = 9;
	//cache_list[4].head = NULL;
	cache_list[4].free = NULL;
	cache_list[5].order = 10;
	//cache_list[5].head = NULL;
	cache_list[5].free = NULL;
	cache_list[6].order = 11;
	//cache_list[6].head = NULL;
	cache_list[6].free = NULL;

}
void init_caches() {
	//need to map it to a physical page, so lets get a frame.
	uint64_t *cache_list_physical = get_free_frames(0);
	cache_t *cache_list = (cache_t *) BASE_MEMORY_MANAGER; // this is the first hardcoded virtual address, we can make it a global variable we should do so.
	//*******add code to map the linear to virtual************
	setup_kernel_page_tables((uint64_t) cache_list,
			(uint64_t) cache_list_physical);
	cache_info.head = cache_list; // this marks the beginning of cache_list it is a global variables. needed to access specific cache information.
	cache_info.current_virtual = (void *) (BASE_MEMORY_MANAGER + 0x10000); //arbitrarily starting the actual virtual addresses at 10 pages apart. We can change it if something comes up
	init_cache_list(cache_list); // this initialized the cache list
	printf("cache location: %p\n", cache_list);
}
//gives the current memory location in virtual space. Also, we increase the virtual address to the next location order distance away.
void *get_virtual_location(int order) {
	void *return_loc = (void *) cache_info.current_virtual;
	cache_info.current_virtual += (0x1000) * (1 << order); //shifting by those many pages.
	return return_loc;
}
//this fucntion will add free_t header to the beginning of every memory location in a page that has been allocated
void prepare_page(int order, void *address) { //order max 11
//	printf("preparing page..\n");
	int i;
	uint64_t num_memory_slots = PAGE_SIZE / (1 << order);
	for (i = 0; i < num_memory_slots - 1; i++) {
		void *current_address = (void *) ((uint64_t) address
				+ (uint64_t) (i * (1 << order)));
		void *next_address = (void *) ((uint64_t) current_address
				+ (uint64_t) (1 << order));
		free_t *free_location = (free_t *) current_address;
		free_location->next_free = next_address;
	}
	void *current_address = (void *) ((uint64_t) address
			+ (uint64_t) (i * (1 << order)));
	free_t *free_location = (free_t *) current_address;
	free_location->next_free = NULL;
	cache_t *cache = cache_info.head + sizeof(cache_t) * (order - 5);
	cache->free = address;
}
// this function gets the actual address of the memory location. It assumes that there are actually free slots in the memory location.
void *get_mem_alloc(int order) {
	cache_t *cache = cache_info.head + sizeof(cache_t) * (order - 5);
	void *return_addr = (void *) (cache->free);
	free_t *free_address = (free_t *) return_addr;
	cache->free = free_address->next_free;
	free_address->next_free = NULL;
	return return_addr;
}
// if the page size if greater than 2048 bytes then get the page directly.
void *get_pages_directly(int order){
	void *free_frame = (void *)get_free_frames(order);
	void *virtual_addr = (void *)get_virtual_location(order);
	void *return_addr = virtual_addr;
	//****** map virtual to physical ***********
	for(uint64_t i = 0; i < (1<<order); i++){
//		printf("%d  ",order);
		setup_kernel_page_tables((uint64_t)virtual_addr, (uint64_t)free_frame);
		virtual_addr = (void *)((uint64_t)virtual_addr + 0x1000);
		free_frame = (void *)((uint64_t)free_frame + 0x1000);
	}
	return return_addr;
}
void *allocate_in_cache(int order) {
	int cache_location = order - 5;
	if (cache_location <= 6) { //less than 2048
		cache_t *cache = cache_info.head + sizeof(cache_t) * cache_location;
		if (cache->free == NULL) { //we need to add a new page, because either 1) it has never been initialized or that there are no more free pages left
			void *free_frame = (void *) get_free_frames(0); //allocating 1 free frame since we only get pages one at a time and max size allocated by this method is 2048
			void *virtual_addr = (void *) get_virtual_location(0);
			//*******add code to map the linear to virtual************
			setup_kernel_page_tables((uint64_t) virtual_addr,
					(uint64_t) free_frame);
			prepare_page(order, virtual_addr); // this function will add headers to each location in the page. free_t
			return get_mem_alloc(order);
		} else {

			return get_mem_alloc(order);
		}
	} else {
		return get_pages_directly(order - 12);
	}
	return NULL;
}
//question what to do with the virtual space vacated by the mem allocation. Should we use it.
void dealloc_in_cache(int order, void *ptr) {
	int cache_location = order - 5;
	if (cache_location <= 6) {
		cache_t *cache = cache_info.head + sizeof(cache_t) * cache_location;
		void *free = cache->free;
		free_t *new_free_node = (free_t *) ptr;
		new_free_node->next_free = free;
		cache->free = ptr;
	} else {
		return_pages((uint64_t) ptr, free_list, order); //is there any change to page table entries ? It shdn't be I am still in kernel space
	}
}
//http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
uint64_t next_power_two(uint64_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	v++;
	return v;
}
uint64_t find_order(uint64_t size) {
	uint64_t number = size;
	int count = 0;
	while (number != 0) {
		count++;
		number = number >> 1;
	}
	return count - 1;
}

void *kmalloc(uint64_t size) {
	uint64_t total_size = sizeof(kmalloc_t) + size;
	total_size = next_power_two(total_size);
	if (total_size & (ALIGNMENT - 1)) {
		total_size = ALIGNMENT;  //making sure the size is minimum of 32
	}
	uint64_t order = find_order(total_size);
	void *addr = allocate_in_cache(order);
	void *return_addr =
			(void *) ((uint64_t) addr + (uint64_t) sizeof(kmalloc_t));
	kmalloc_t *header = (kmalloc_t *) addr;
	header->order = order;
	return return_addr;
}

void kfree(void *addr) {
	kmalloc_t *header = (kmalloc_t *) ((uint64_t) addr
			- (uint64_t) sizeof(kmalloc_t));
	uint64_t order = header->order;
	header->order = 0;
	dealloc_in_cache(order, (void *) header);
}


int is_kernel_addr(uint64_t addr) {
	return do_pmls_clash(addr,BASE_MEMORY_MANAGER);
}

