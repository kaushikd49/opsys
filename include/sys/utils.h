// reverse mapping of addrs of page table pages
// need this since parent process has to create
// page tables of child without switching cr3
typedef struct phys_virt_mapping {
	uint64_t physical_addr;
	uint64_t virtual_addr;
	int isset;
	struct phys_virt_mapping * next;
} pv_map_t;

int if_not_contains_pv_mapping(pv_map_t* pv_map_node, uint64_t page_base,
		uint64_t page_phys_addr);

void cache_pv_mapping(pv_map_t* pv_map_node, uint64_t page_base,
		uint64_t page_phys_addr);

pv_map_t* init_pv_map();

void free_pv_map(pv_map_t* pv_map_node);
