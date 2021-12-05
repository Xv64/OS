#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "x86.h"

void sleep_forever();

void main(void){
	if (!amblessed()) {
		fprintf(stdout, "Not blessed, exiting\n");
		procexit();
	}
    int procs = nprocs();
	int pid;
	int i = 0;
	while(1) {
	    for(; i < procs; i++) {
	        pid = bfork();
	        if(pid == 0) {
	            break;
	        }
			setpriority(pid, 0); // set kidle worker to lowest scheduling priority
	    }
		if(pid == 0) {
	    	sleep_forever();
		} else {
			// wait for a child to die.
			wait();
			// when/if they do, increment procs so we spawn one more
			procs++;
		}
	}
}

void sleep_forever() {
    while(1) {
        cpuhalt();
	}
}
