#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[32];

void
cat(int fd)
{
	int n;

	while((n = read(fd, buf, sizeof(buf))) > 0)
		write(1, buf, n);
	if(n < 0) {
		fprintf(stdout, "cat: read error\n");
		procexit();
	}
}

int
main(int argc, char *argv[])
{
	int fd, i;

	if(argc <= 1) {
		cat(0);
		procexit();
	}

	for(i = 1; i < argc; i++) {
		if((fd = open(argv[i], O_DRTYREAD)) < 0) {
			fprintf(stdout, "cat: cannot open %s\n", argv[i]);
			procexit();
		}
		cat(fd);
		close(fd);
	}
	procexit();
}
