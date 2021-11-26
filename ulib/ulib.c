#include <stdarg.h>

#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "console.h"

//defining these k-level syscalls here for use in ioctl
void kconsole_info(struct winsize *winsz);
//end

char*
gets(char *buf, int max)
{
	int i, cc;
	char c;

	for(i=0; i+1 < max; ) {
		cc = read(0, &c, 1);
		if(cc < 1)
			break;
		buf[i++] = c;
		if(c == '\n' || c == '\r')
			break;
	}
	buf[i] = '\0';
	return buf;
}

int
stat(char *n, struct stat *st)
{
	int fd;
	int r;

	fd = open(n, O_DRTYREAD);
	if(fd < 0)
		return -1;
	r = fstat(fd, st);
	close(fd);
	return r;
}

int32 ioctl(int32 fd, uint64 cmd, ...){
	va_list ap;
	va_start(ap, cmd); //get additional args

	if(cmd == TIOCGWINSZ) {
		//this is really the only call we support right now
		struct winsize *winsz = va_arg(ap, struct winsize *);
		kconsole_info(winsz);
		return 0;
	}
	return -1;
}

char *ttyname(int32 fd){
	//this is a BASIC implementation
	if(fd == stdout) {
		//TODO: what terminal are they on? what device does this map to?
		//none of these questions are answered (nor implemented), but
		//need to be in a proper impl.
		return "/dev/tty0"; //for now, let's fudge it.
	}
	return 0;
}
