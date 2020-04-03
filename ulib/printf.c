#include <stdarg.h>

#include "types.h"
#include "stat.h"
#include "user.h"

//printf family of functions
//defined on page 404 of POSIX Base Definitions, Issue 6

#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#define PRINT_SCREEN 0
#define PRINT_BUFFER 1

static void putc(int fd, char c) {
	write(fd, &c, 1);
}

static void printint(int fd, int xx, int base, int sgn) {
	static char digits[] = "0123456789ABCDEF";
	char buf[16];
	int i, neg;
	uint x;

	neg = 0;
	if(sgn && xx < 0) {
		neg = 1;
		x = -xx;
	} else {
		x = xx;
	}

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while((x /= base) != 0);
	if(neg)
		buf[i++] = '-';

	while(--i >= 0)
		putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
static void vprintf(uint8 mode, int32 fd, char *UNUSED(buf), uint32 UNUSED(maxlen), char *fmt,  va_list ap){
	char *s;
	int c, i, state;

    state = 0;
	for(i = 0; fmt[i]; i++) {
		c = fmt[i] & 0xff;
		if(state == 0) {
			if(c == '%') {
				state = '%';
			} else {
                if(mode == PRINT_SCREEN){
    				putc(fd, c);
                }else{
                    //TODO
                }
			}
		} else if(state == '%') {
			if(c == 'd') {
                if(mode == PRINT_SCREEN){
    				printint(fd, va_arg(ap, int), 10, 1);
                }else{
                    //TODO
                }
			} else if(c == 'x' || c == 'p') {
                if(mode == PRINT_SCREEN){
    				printint(fd, va_arg(ap, int), 16, 0);
                }else{
                    //TODO
                }
			} else if(c == 's') {
				s = va_arg(ap, char*);
				if(s == 0)
					s = "(null)";
				while(*s != 0) {
                    if(mode == PRINT_SCREEN){
        				putc(fd, *s);
                    }else{
                        //TODO
                    }
					s++;
				}
			} else if(c == 'c') {
                if(mode == PRINT_SCREEN){
                    putc(fd, va_arg(ap, uint));
                }else{
                    //TODO
                }
			} else if(c == '%') {
                if(mode == PRINT_SCREEN){
                    putc(fd, c);
                }else{
                    //TODO
                }
			} else {
				// Unknown % sequence.  Print it to draw attention.
                if(mode == PRINT_SCREEN){
                    putc(fd, '%');
    				putc(fd, c);
                }else{
                    //TODO
                }
			}
			state = 0;
		}
	}
}

void fprintf(int32 fd, char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintf(PRINT_SCREEN, fd, 0, 0, fmt, args);
    va_end(args);
}

void printf(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(PRINT_SCREEN, 0, 0, 0, fmt, args);
    va_end(args);
}

int snprintf(char *UNUSED(s), unsigned int UNUSED(n), const char *UNUSED(fmt), ...) {
    return -1;
}
