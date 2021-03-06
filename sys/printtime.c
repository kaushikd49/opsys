#include<sys/isr_stuff.h>
#include<sys/process.h>
#include <sys/sbunix.h>
#include<sys/nanosleep_functions.h>

int seconds_boot = 0;
int ms_boot = 0;
uint64_t temp_print_time(uint64_t stack_top) {
	// with a frequency of 18.2065 Hz, a interrupt is sent every .0549254 seconds so a second happens every 18.2 calls.

// static int lost_precision = 0; //for the .2 so every 10 increment increment ms_boot once more
	//printf("%x", time);
// lost_precision++;
	ms_boot = ms_boot + 1;
// if(lost_precision == 9) //can optimize
//		ms_boot++;
	uint64_t returnval;
	if (ms_boot < 18) {
			remove_nanosleep_list(seconds_boot, ms_boot);
			{
				returnval = temp_preempt(stack_top);
//				printf("preempt over-- ");
			}

		return returnval;
	} else {
		ms_boot = ms_boot % 2; //can optimize
		seconds_boot = seconds_boot + 1;
		printHexIntTime(seconds_boot);
	}
	return stack_top;
}
