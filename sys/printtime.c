#include<sys/isr_stuff.h>
#include<sys/process.h>
#include <sys/sbunix.h>
void print_time(uint64_t stack_top) {
	// with a frequency of 18.2065 Hz, a interrupt is sent every .0549254 seconds so a second happens every 18.2 calls.
	static int seconds_boot = 0;
	static int ms_boot = 0;
// static int lost_precision = 0; //for the .2 so every 10 increment increment ms_boot once more
	//printf("%x", time);
// lost_precision++;
	ms_boot = ms_boot + 1;
// if(lost_precision == 9) //can optimize
//		ms_boot++;

	if (ms_boot < 18) {
			preempt(stack_top);

		return;
	} else {
		ms_boot = ms_boot % 2; //can optimize
		seconds_boot = seconds_boot + 1;
		printHexIntTime(seconds_boot);
	}

}
