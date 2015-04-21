#include<sys/process.h>
#include<sys/scheduling.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include<sys/isr_stuff.h>
#include <sys/kmalloc.h>
//void test_main(){
//	uint64_t i = 1;
//	printf("inside kernel thread %d", i);
//	limit = limit << 2;
//	while(i < limit){
//		//if(i%100000000 == 0){
//		//	printf("%d ", i);
//		//	i++;
//		//}
//		i++;
//	}
//	quit_kernel_thread();
//}

void waiting_to_running_q(){

	while(1){
		task_struct_t *current_waiting = waitingtask;
	do{

		if(current_waiting != NULL && current_waiting->p_state == STATE_READY){
			__asm__ __volatile__("cli");
			printf("\n moving back bitchas");
			move_process_waitq_to_runq(current_waiting->pid);

			current_waiting = waitingtask;

		}
		if(current_waiting !=NULL)
			current_waiting = current_waiting->next;
	}while(waitingtask !=NULL && current_waiting != waitingtask);
		__asm__ __volatile__("sti");
	}
}


void read_thread_process(int fd, void *buffer, uint64_t size){
	file_desc_t *current_fd = currenttask->filearray[fd];
	while(current_fd->busy == 1 && current_fd->current_process !=currenttask->pid);
	__asm__ __volatile__("cli");
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
	__asm__ __volatile__("sti");
	while(current_fd->ready == 0);
	__asm__ __volatile__("cli");
	char *current_pointer = currenttask->filearray[fd]->current_pointer;
		char *end_file = current_pointer + currenttask->filearray[fd]->size;
		int copied = 0;
		char *buff = (char *)buffer;
		while(copied < size && current_pointer < end_file ){
			buff[copied] = *current_pointer;
			current_pointer++;
			currenttask->filearray[fd]->current_pointer++;
			copied++;
		}
		int ppid = currenttask->ppid;
		task_struct_t *current_waiting = waitingtask;
		do{
			if(current_waiting!=NULL){
				if(current_waiting->pid == ppid){
					regs_syscall_t *regs = (regs_syscall_t *)(current_waiting->state.kernel_rsp);
					regs->rax = copied;
					current_waiting->p_state = STATE_READY;
					break;
				}
			}
			if(current_waiting!=NULL)
				current_waiting = current_waiting->next;
		}while(current_waiting!=NULL);
		current_fd->busy = 0;
		printf("busy done");
		quit_kernel_thread();
	//	return copied;

}
