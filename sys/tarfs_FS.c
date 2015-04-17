//#include<sys/defs.h>
//#include<sys/sbunix.h>
//#include <sys/kmalloc.h>
//#include<sys/kmalloc.h>
//#include<sys/tarfs.h>
//void find_file_tarfs(){
//	struct posix_header_ustar *current =
//				(struct posix_header_ustar *) &_binary_tarfs_start;
//	while ((uint64_t) current < (uint64_t) (&_binary_tarfs_end)) {
//			printf("%s  ", current->name)
//			//	printf("elf header: %x\n",*(current + (uint64_t)sizeof(struct posix_header_ustar)));
//			uint64_t header_next = (uint64_t) ((align(
//					convert_ocatalstr_todecimal(current->size), TARFS_ALIGNMENT))
//					+ sizeof(struct posix_header_ustar) + (uint64_t) current);
//	//		printf("header : %x", header_next);
//			current = (struct posix_header_ustar *) (header_next);
//
//		}
//}
////void init_tarfs(){
////
////
////}
