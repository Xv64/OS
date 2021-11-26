// init: The initial user-level program

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

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

	for(;;) {

		spawn("/kexts/kzero", "kzero", 1);
		spawn("/kexts/krandom", "krandom", 1);
		spawn("/bin/sh", "sh", 0);

		while(1) {
			sleep(30);
		}
		// while((wpid=wait()) >= 0 && wpid != pid)
		//   fprintf(stdout, "zombie!\n");
	}
}
