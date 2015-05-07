#include <sys/defs.h>
#include <sys/utils.h>
#include <sys/kmalloc.h>
#include <sys/paging.h>

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

void cache_pv_mapping(pv_map_t* pv_map_node, uint64_t virt_addr,
		uint64_t page_phys_addr) {
	if (pv_map_node->isset) {
		pv_map_t * temp = kmalloc(sizeof(pv_map_t));
		temp->physical_addr = page_phys_addr;
		temp->virtual_addr = virt_addr;
		temp->next = pv_map_node->next;
		pv_map_node->next = temp;
	} else {
		pv_map_node->isset = 1;
		pv_map_node->physical_addr = page_phys_addr;
		pv_map_node->virtual_addr = virt_addr;
		pv_map_node->next = NULL;
	}
}

int if_not_contains_virt_addr(pv_map_t* pv_map_node, uint64_t virt_addr,
		uint64_t page_phys_addr) {
	for (pv_map_t* temp = pv_map_node; temp != NULL; temp = temp->next) {
		if (temp->virtual_addr == virt_addr)
			return 0;
	}
	return 1;
}

int if_not_contains_phys_addr(pv_map_t* pv_map_node, uint64_t virt_addr,
		uint64_t page_phys_addr) {
	for (pv_map_t* temp = pv_map_node; temp != NULL; temp = temp->next) {
		if (temp->physical_addr == page_phys_addr)
			return 0;
	}
	return 1;
}

uint64_t* phys_to_virt_map(uint64_t* physaddr, void * pv_map) {
	//	uint64_t vaddr = NULL;
	//	int is_found = phys_to_virt(next_entity_base_phys, (pv_map_t) pv_map,
	//			&vaddr);
	//	if (is_found) {
	//		return (uint64_t*) *vaddr;
	//	} else {
	//		// map the phys addr to a virtual one and cache it in pv_map
	//		uint64_t res_addr = (uint64_t) get_virtual_location_cr3(0);
	//		insert_into_map(pv_map, next_entity_base_phys, res_addr);
	//		return res_addr;
	//	}

	// TODO: VERY IMPORTANT: give back the virt addr so that the kernel can
	// reuse this virt addr, else kernel page table pages will increase
	// very quickly and since we are not freeing kernel page table pages.

	uint64_t res_addr = (uint64_t) get_virtual_location(0);
	setup_page_tables_after_cr3_update(res_addr, (uint64_t) physaddr, 1, 1, 0);
	// remember to give back this virtual address
	return (uint64_t *) res_addr;

}

void unmap_vaddr(uint64_t SPARE_ADDR) {
	// now unmap the page, SPARE's job is done
	uint64_t* pte = virtual_addr_pte(SPARE_ADDR);
	*pte = 0;
}

