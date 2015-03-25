//#include<sys/defs.h>
//#include<sys/sbunix.h>
//
//typedef struct cache_init{
//	struct cache_t *head;
//}cache_init;
//typedef struct cache_t{
//	int order;
//	cache_page *head;
//	free_t *free;
//}cache_t;
//typedef struct free_t{
//	void *next_free;
//}free_t;
//cache_init cache_init = { NULL};
//void init_caches(uint64_t addr);
//void init_cache_list();
//void init_cache_list(cache_t *cache_list){
//	cache_list[0].order = 5;
//	cache_list[0].head = NULL;
//	cache_list[0].free = NULL;
//	cache_list[1].order = 6;
//	cache_list[1].head = NULL;
//	cache_list[1].free = NULL;
//	cache_list[2].order = 7;
//	cache_list[2].head = NULL;
//	cache_list[2].free = NULL;
//	cache_list[3].order = 8;
//	cache_list[3].head = NULL;
//	cache_list[3].free = NULL;
//	cache_list[4].order = 9;
//	cache_list[4].head = NULL;
//	cache_list[4].free = NULL;
//	cache_list[5].order = 10;
//	cache_list[5].head = NULL;
//	cache_list[5].free = NULL;
//	cache_list[6].order = 11;
//	cache_list[6].head = NULL;
//	cache_list[6].free = NULL;
//
//
//}
//void init_caches(uint64_t addr){
//	cache_t *cache_list = (cache_t *)addr;
//	init_cache_list(cache_list);
//
//}
