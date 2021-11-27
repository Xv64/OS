#include <stdarg.h>

#include "platform.h"
#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "console.h"

#define PRINT_SCREEN 0
#define PRINT_BUFFER 1

static int32 putc(int fd, char c) {
	write(fd, &c, 1);
	return 1;
}

static int8 printint(int xx, int base, int sgn, char *outbuf) {
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
	int32 len = i;

	while(--i >= 0) {
		outbuf[len - (i + 1)] = buf[i];
	}

	return len;
}

int isspace(int c) {
	//"The isspace( ) function shall return non-zero if c is a white-space character;
	//otherwise, it shall return 0."  - POSIX Base Spec, Issue 6 page 647
	char chr = (char)c;
	if(chr == ' ' || chr == '\t' || chr == '\v' || chr == '\n' || chr == '\f' || chr == '\r') {
		//the above is the list of all ASCII-space whitespace character codes
		//see: https://en.wikipedia.org/wiki/Whitespace_character#Unicode
		return 1;
	}
	return 0;
}


int isdigit(int c) {
	//The isdigit ( ) function shall return non-zero if c is a decimal digit; otherwise, it shall return 0.
	//- POSIX Base Spec, Issue 6 page 632
	if(c >= 48 && c <= 57) {
		//ASCII 48 -> 57 (inclusive) are digits 0-9
		return 1;
	}
	return 0;
}


long strtol(const char *restrict str, char **restrict endptr, int base) {
	//POSIX Base Definitions, Issue 6 - page 1456
	const char *p = str;

	while(isspace( (int)*p )) {
		p++;
	}

	int n = 0;
	while(isdigit( (int)*p )) {
		n = n*10 + (int)*p - '0';
		p++;
	}
	endptr = (char **) &p;
	return n;
}

int byteorder (void) {
    uint32 x = 0x00000001;
    return *(uint8 *)&x ? __LITTLE_ENDIAN : __BIG_ENDIAN;
}

static int endian;
uint16 ntoh16 (uint16 n) {
    if (!endian)
        endian = byteorder();
    return endian == __LITTLE_ENDIAN ? byteswap16(n) : n;
}

uint16 hton16 (uint16 h) {
    if (!endian)
        endian = byteorder();
    return endian == __LITTLE_ENDIAN ? byteswap16(h) : h;
}


// This is the core print method, all external functions delegate to this.
static int32 vprintf(uint8 mode, int32 fd, char *buf, uint32 maxlen, const char *fmt,  va_list ap){
	char *s;
	int c, i, state;
	int32 len = 0; //based on mode this will either represent:
	               //     when mode == PRINT_SCREEN: the total number of characters printed
	               //     when mode == PRINT_BUFFER: the total number of characters that *could* have been written

	state = 0;
	for(i = 0; fmt[i]; i++) {
		c = fmt[i] & 0xff;
		if(state == 0) {
			if(c == '%') {
				state = '%';
			} else {
				if(mode == PRINT_SCREEN) {
					len += putc(fd, c);
				}else{
					if(len < maxlen) {
						buf[len] = c;
					}
					len++;
				}
			}
		} else if(state == '%') {
			if(c == 'd') {
				char buf[16];
				int8 segmentLen = printint(va_arg(ap, int), 10, 1, &buf[0]);
				for(uint8 j = 0; j != segmentLen; j++) {
					if(mode == PRINT_SCREEN) {
						len += putc(fd, buf[j]);
					}else {
						if(len < maxlen) {
							buf[len] = c;
						}
						len++;
					}
				}
			} else if(c == 'x' || c == 'p') {
				char buf[16];
				int8 segmentLen = printint(va_arg(ap, int), 16, 0, &buf[0]);
				for(uint8 j = 0; j != segmentLen; j++) {
					if(mode == PRINT_SCREEN) {
						len += putc(fd, buf[j]);
					}else {
						if(len < maxlen) {
							buf[len] = c;
						}
						len++;
					}
				}
			} else if(c == 's') {
				s = va_arg(ap, char*);
				if(s == 0)
					s = "(null)";
				while(*s != 0) {
					if(mode == PRINT_SCREEN) {
						len += putc(fd, *s);
					}else{
						if(len < maxlen) {
							buf[len] = *s;
						}
						len++;
					}
					s++;
				}
			} else if(c == 'c') {
				if(mode == PRINT_SCREEN) {
					len += putc(fd, va_arg(ap, uint));
				}else{
					if(len < maxlen) {
						buf[len] = va_arg(ap, uint);
					}
					len++;
				}
			} else if(c == '%') {
				if(mode == PRINT_SCREEN) {
					len += putc(fd, c);
				}else{
					if(len < maxlen) {
						buf[len] = c;
					}
					len++;
				}
			} else {
				// Unknown % sequence.  Print it to draw attention.
				if(mode == PRINT_SCREEN) {
					len += putc(fd, '%');
					len += putc(fd, c);
				}else{
					if(len < maxlen) {
						buf[len] = '%';
					}
					len++;
					if(len < maxlen) {
						buf[len] = c;
					}
					len++;
				}
			}
			state = 0;
		}
	}
	if(mode == PRINT_BUFFER) {
		//null terminate our string, but do NOT increment len in the process
		buf[ len < maxlen ? len + 1 : maxlen ] = '\0';
	}
	return len;
}

int snprintf(char *s, unsigned int n, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vprintf(PRINT_BUFFER, 0, s, n, fmt, args);
	va_end(args);
	return result;
}

int strncmp(const char* p, const char* q, uint n) {
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	return (uchar) * p - (uchar) * q;
}
