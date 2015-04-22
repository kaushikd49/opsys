#include<sys/process.h>
#include<sys/scheduling.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include<sys/isr_stuff.h>
#include <sys/kmalloc.h>
#include<sys/tarfs_FS.h>
#include<sys/kernel_thread.h>
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
void noop(int i){

}
inline void fake_preempt(){
	__asm__ __volatile__("int $99");

}
void waiting_to_running_q(){

	while(1){
		__asm__ __volatile__("cli");
		volatile task_struct_t *current_waiting = waitingtask;
	do{

		if(current_waiting != NULL && current_waiting->p_state == STATE_READY){

			printf("\n moving back bitchas");
			move_process_waitq_to_runq(current_waiting->pid);

			current_waiting = waitingtask;

		}
		if(current_waiting !=NULL)
			current_waiting = current_waiting->next;
	}while(waitingtask !=NULL && current_waiting != waitingtask);
		__asm__ __volatile__("sti");
//		for(int i = 0; i<1000; i++){
//				noop(i);
//			}
		fake_preempt();
	}
}
int check_ready(file_desc_t *current_fd){
	return current_fd->ready;
}
int check_busy(file_desc_t *current_fd){
	return current_fd->busy;
}
void read_thread_process(int fd, void *buffer, uint64_t size){
	file_desc_t *current_fd = currenttask->filearray[fd];
	while(check_busy(current_fd) == 1 && current_fd->current_process !=currenttask->pid);
	__asm__ __volatile__("cli");
	current_fd->busy = 1;
	current_fd->current_process = currenttask->pid;
	__asm__ __volatile__("sti");
//	int ready_check = 0;
	while(check_ready(current_fd) == 0);

//	__asm__ __volatile__("b:cmp $1, %0\n\t"
//						 " jn 	e b\n\t"
//						 :
//						 :"r"(current_fd->ready));
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
		if(fd == 0)
			clear_keyboard_busy();
		current_fd->ready = 0;
		current_fd->busy = 0;
		quit_kernel_thread();
	//	return copied;

}

//clears keyoard buffer when a new process begins writing
void clear_keyboard_busy(){
	char *current = (char *)(input_buffer);
	for(uint64_t i = 0; i < 0x1000 ; i++){
		*current = 0;
		current++;
	}
	currenttask->filearray[0]->current_pointer = (char *)input_buffer;

}

void find_child(task_struct_t *temp){

	int ppid = temp->pid;
	int wait_for = temp->waiting_for;
	if(wait_for == -1){
		task_struct_t *temp2 = waitingtask;
		do{
			if(temp2->is_kernel_process == 0 && temp2->ppid == ppid){
				return;
			}
			temp2 = temp2->next;
		}while(temp2!=waitingtask);
		temp2 = currenttask;
		do{
					if(temp2->is_kernel_process == 0 && temp2->ppid == ppid){
						return;
					}
					temp2 = temp2->next;
		}while(temp2!=currenttask);
	}
	else{
		task_struct_t *temp2 = waitingtask;
				do{
					if(temp2->is_kernel_process == 0 && temp2->pid == wait_for){
						return;
					}
					temp2 = temp2->next;
				}while(temp2!=waitingtask);
				temp2 = currenttask;
				do{
							if(temp2->is_kernel_process == 0 && temp2->pid == wait_for){
								return;
							}
							temp2 = temp2->next;
				}while(temp2!=currenttask);
	}
	printf("daemon activated");
	temp->p_state = STATE_READY;
}

void check_user_process_waitpid_daemon(){


	while(1){
	__asm__ __volatile__("cli");
	task_struct_t *temp = waitingtask;

	do{

		if(temp!=NULL && temp->is_kernel_process == 0 && temp->waiting_for != 999){
			find_child(temp);
		}
		if(temp!=NULL)
			temp = temp->next;
	}while(temp!=waitingtask);
	__asm__ __volatile__("sti");
//	for(int i = 0; i<1000000; i++){
//		noop(i);
//	}
	fake_preempt();
	}
}
