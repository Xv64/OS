// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

int spawn(char *task, char *name, int blessed){
	fprintf(stdout, "init: starting %s\n", name);
	int pid = blessed ? bfork() : fork();
	char *argv[] = { name, 0 };
	if(pid < 0) {
		fprintf(stdout, "init: fork failed\n");
		procexit();
	}
	if(pid == 0) {
		exec(task, argv);
		fprintf(stdout, "init: exec %s failed\n", name);
		procexit();
	}
	return pid;
}

int main(void) {
	if(open("/dev/tty0", O_RDWR) < 0) {
		mknod("/dev/tty0", 1, 1);
		open("/dev/tty0", O_RDWR);
	}
	dup(0); // stdout
	dup(0); // stderr

	int kzeropid = 0;
	int krandompid = 0;
	int kidlepid = 0;
	int shpid = 0;
	int child = 0;
	fprintf(stdout, "init: starting...\n");
	while(1) {

		kzeropid = child == kzeropid ? spawn("/kexts/kzero", "kzero", 1) : kzeropid;
		krandompid = child == krandompid ? spawn("/kexts/krandom", "krandom", 1) : krandompid;
		kidlepid = child == kidlepid ? spawn("/kexts/kidle", "kidle", 1) : kidlepid;
		shpid = child == shpid ? spawn("/bin/sh", "sh", 0) : shpid;

		sleep(10);
		child = wait();
		write(1, "--", 2);
		fprintf(stdout, "init: pid %d died - respawning\n", child);
	}
}
