#include <sys/defs.h>
#include <sys/utils.h>
#include <sys/kmalloc.h>

pv_map_t* init_pv_map() {
	pv_map_t* pv_map_node = kmalloc(sizeof(pv_map_t));
	pv_map_node->isset = 0;
	pv_map_node->next = NULL;
	return pv_map_node;
}

void free_pv_map(pv_map_t* pv_map_node) {
	pv_map_t* p = pv_map_node;
	pv_map_t* q = NULL;

	while (p != NULL) {
		if (q != NULL) {
			kfree(q);
		}
		p = p->next;
		q = p;
	}

	if (q != NULL) {
		kfree(q);
	}
}

void cache_pv_mapping(pv_map_t* pv_map_node, uint64_t page_base,
		uint64_t page_phys_addr) {
	if (pv_map_node->isset) {
		pv_map_t * temp = kmalloc(sizeof(pv_map_t));
		temp->physical_addr = page_phys_addr;
		temp->virtual_addr = page_base;
		temp->next = pv_map_node->next;
		pv_map_node->next = temp;
	} else {
		pv_map_node->isset = 1;
		pv_map_node->physical_addr = page_phys_addr;
		pv_map_node->virtual_addr = page_base;
		pv_map_node->next = NULL;
	}
}

int if_not_contains_pv_mapping(pv_map_t* pv_map_node, uint64_t page_base,
		uint64_t page_phys_addr) {
	for (pv_map_t* temp = pv_map_node; temp != NULL; temp = temp->next) {
		if (temp->virtual_addr == page_base)
			return 0;
	}
	return 1;
}
