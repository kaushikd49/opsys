#include <stdarg.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include <sys/paging.h>
#include<sys/kmalloc.h>
#include<sys/process.h>

extern task_struct_t *currenttask;//points to the head of the running list
//extern static task_struct_t *waitingtask;//
void add_process_waitq(task_struct_t *waiting_process){
	if(waitingtask == NULL){
		waitingtask = waiting_process;//both links of the waiting process point to itself
		return;
	}
	task_struct_t *current = waitingtask;
	while(current->next!= waitingtask){
		current = current->next;
	}
	waiting_process->next = current->next;
	current->next = waiting_process;
}

void add_process_runq(task_struct_t *runnable_process){
	if(currenttask == NULL){//will never happen
		currenttask = runnable_process;
	}
	task_struct_t *current = currenttask;
	while(current->next !=currenttask){
		current = current->next;
	}
	runnable_process->next = current->next;
	current->next = runnable_process;
}

task_struct_t *remove_process_waitq(uint64_t pid){
	if(waitingtask == NULL){
		return NULL;
	}
	if(waitingtask->pid == pid){
		task_struct_t *return_taskstruct = waitingtask;
		if(waitingtask->next == waitingtask){
			waitingtask = NULL;
		}
		else{
			task_struct_t *current = waitingtask;
			while(current->next != waitingtask){
				current = current->next;
			}
			current->next = current->next->next;
			waitingtask = waitingtask->next;
		}
		return_taskstruct->next = return_taskstruct;
		return return_taskstruct;
	}
	task_struct_t *prev = waitingtask;
	task_struct_t *current = waitingtask->next;
	while(current!=waitingtask){
		if(current->pid == pid){
			prev->next = current->next;
			current->next = current;
					return current;
		}
		prev = current;
		current = current->next;
	}
	return NULL;
}

task_struct_t *remove_process_runq(uint64_t pid){
	if(currenttask == NULL){
		return NULL;
	}
	if(currenttask->pid == pid){
		task_struct_t *return_taskstruct = currenttask;
		if(currenttask->next == currenttask){
			currenttask = NULL;
		}
		else{
			task_struct_t *current = currenttask;
			while(current->next != currenttask){
				current = current->next;
			}
			current->next = current->next->next;
			currenttask = currenttask->next;
		}
		return_taskstruct->next = return_taskstruct;
		return return_taskstruct;
	}
	task_struct_t *prev = currenttask;
	task_struct_t *current = currenttask->next;
	while(current!=currenttask){
		if(current->pid == pid){
			prev->next = current->next;
			current->next = current;
					return current;
		}
		prev = current;
		current = current->next;
	}
	return NULL;
}

void move_process_waitq_to_runq(uint64_t pid){
//	printf("%d w to r\n", pid);
	__asm__ __volatile__("cli");
	task_struct_t *temp = remove_process_waitq(pid);
	temp->p_state = STATE_RUNNING;
	if(temp!=NULL)
		add_process_runq(temp);
	__asm__ __volatile__("sti");
}
void move_process_runq_to_waitq(uint64_t pid){
	__asm__ __volatile__("cli");
	task_struct_t *temp = remove_process_runq(pid);
	if(temp!=NULL)
		add_process_waitq(temp);
	__asm__ __volatile("sti");
}


