#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

char *statename(enum procstate state) {
	if (state == EMBRYO) {
		return "EMBRYO";
	} else if (state == SLEEPING) {
		return "SLEEPING";
	} else if (state == RUNNABLE) {
		return "RUNNABLE";
	} else if (state == RUNNING) {
		return "RUNNING";
	} else if (state == ZOMBIE) {
		return "ZOMBIE";
	}
	return "";
}

int main(int argc, char *argv[]) {
	char *name = malloc(32 * sizeof(char));

	fprintf(stdout, "PID\tNAME\tSTATE\n");
	for(uint64 pid = 0; pid != NPROC; pid++) {
		enum procstate state = pstate(pid);
		if (state != UNUSED) {
			char *statestr = statename(state);
			pname(pid, name, 32);
			fprintf(stdout, "%d\t%s\t%s (%d)\n", pid, name, statestr, state);
		}
	}
	procexit();
}
