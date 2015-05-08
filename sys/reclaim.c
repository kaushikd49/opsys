#include<sys/system_calls.h>
#include<sys/sbunix.h>
#include<sys/process.h>
#include<sys/freelist.h>
#include<sys/isr_stuff.h>
#include <sys/kernel_thread.h>

int is_mem_not_enough() {
	// zeroed pages, total free pages threshold
	return (num_free_pages(0) <= 300 || num_free_pages(2) <= 32066);
}

int has_same_cr3(task_struct_t * usr_process, task_struct_t *kprocess) {
	return usr_process->state.cr3 == kprocess->state.cr3;
}

void kill_friend_kthread(task_struct_t *usr_process, task_struct_t * queue) {
	int visited2 = 0;
	for (task_struct_t *u = queue; u != queue || visited2 == 0; u = u->next) {
		visited2 = 1;
		if (u->is_kernel_process == 1 && has_same_cr3(usr_process, u)) {
			printf("Killing kernel process %d\n", u->pid);
			kill_from_queue(u->pid, queue);
		}
	}
}

int kill_and_reclaim(task_struct_t *queue) {
	int visited = 0;
	int any_killed = 0;

	if (queue != NULL) {
		for (task_struct_t *t = queue; t != queue || visited == 0; t =
				t->next) {
			visited = 1;
			if (t != currenttask && t->is_kernel_process == 0 && t->pid > 6) {
				task_struct_t *usr_process = t;
				printf("Killing user process %d\n", usr_process->pid);
				kill_from_queue(t->pid, queue);	// kill user process

				// kill the kernel process on waitQ with same cr3
				kill_friend_kthread(usr_process, waitingtask);
				// kill the kernel process on runQ with same cr3
				kill_friend_kthread(usr_process, currenttask);
				any_killed = 1;
				break;
			}
		}
	}
	return any_killed;
}
task_struct_t *get_waiting() {
	return waitingtask;
}
task_struct_t *get_running() {
	return currenttask;
}
void find_group_in_waitq(uint64_t cr3) {
	task_struct_t *current = waitingtask;
	task_struct_t *wait = waitingtask;
	do {
		if (current != NULL && current->p_state != STATE_TERMINATED
				&& current->state.cr3 == cr3) {
			mark_as_terminated_w(current);
		}
		if (current != NULL)
			current = current->next;
		wait = get_waiting();
	} while (current != NULL && current != wait);
}
void find_group_in_runq(uint64_t cr3) {
	task_struct_t *current = currenttask;
	task_struct_t *wait = currenttask;
	do {
		if (current != NULL && current != currenttask
				&& current->state.cr3 == cr3) {
			task_struct_t *last = current;
			current = current->next;
			mark_as_terminated(last);
		} else if (current != NULL) {
			current = current->next;
		}
		wait = get_running();
	} while (current != NULL && current != wait);
}
int terminate_from_waitq() {
	int ret = 0;
	task_struct_t *current = waitingtask;
	task_struct_t *wait = waitingtask;
	do {
		if (current != NULL && current->pid > 6
				&& current->state.cr3 != currenttask->state.cr3
				&& current->p_state != STATE_TERMINATED) {
			printf("enteredd\n");
			mark_as_terminated_w(current);
			ret = 1;
			find_group_in_waitq(current->state.cr3);
			find_group_in_runq(current->state.cr3);
			break;
		}
		if (current != NULL)
			current = current->next;
		wait = get_waiting();
	} while (current != NULL && current != wait);
	return ret;
}
void reclaim_resource_if_needed() {
	if (is_mem_not_enough()) {
		clean_up_process_body();
//		if(is_mem_not_enough()){
//			if (!kill_and_reclaim(waitingtask))
//				kill_and_reclaim(currenttask);
//		}
		int ret = terminate_from_waitq();
		if (ret == 0) {
			printf("could not deallocate from waitq");
		} else {
			printf("cleaning");
			clean_up_process_body();
		}
	}
}
