#include<sys/nanosleep_functions.h>
extern int seconds_boot;
extern int ms_boot;

nanosleep_node_t *nanosleep_head = NULL;

nanosleep_node_t *make_nanosleep_node(const struct timespec *rqtp,task_struct_t *task){
	nanosleep_node_t *new_node = kmalloc(sizeof(nanosleep_node_t));
	new_node->seconds = seconds_boot + rqtp->tv_sec;
	uint64_t ms = seconds_boot +rqtp->tv_nsec;
	ms = (ms*17/100);
	new_node->ms = ms;
	task->p_state = STATE_WAITING;
	new_node->task = task;
	new_node->next = NULL;
	return new_node;
}
void add_nanosleep_list(nanosleep_node_t *node){
	if(nanosleep_head == NULL){
		nanosleep_head = node;
		return;
	}
	nanosleep_node_t *current = nanosleep_head;
	while(current->next!=NULL){
		current = current->next;
	}
	current->next = node;
	return;
}


void remove_nanosleep_list(uint64_t seconds, uint64_t ms){
	nanosleep_node_t *current = nanosleep_head;
	nanosleep_node_t *prev = NULL;
	while(current!=NULL){
		if(current->seconds == seconds && (current->ms == ms-1 || current->ms == ms || current->ms == ms+1)){
			if(prev == NULL){
				current->task->p_state = STATE_READY;
				nanosleep_head = current->next;
				kfree(current);
				current = nanosleep_head;

			}
			else{
				current->task->p_state = STATE_READY;
				prev->next = current->next;
				kfree(current);
				current = prev->next;
			}
		}
		else{
			prev = current;
			current = current->next;
		}
	}
}

