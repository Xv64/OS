#include "syscalls.h"
#include "stdio.h"
#include "fcntl.h"

int  fgetc(FILE *stream) {
    if(stream->readable == -1){
        return EOF;
    }
    unsigned char buf = EOF;
    stream->readable = read(stream->fd, &buf, sizeof(buf));
    if(stream->readable == -1){
        return EOF;
    }
    return (int)buf;
}


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

	while(--i >= 0){
		outbuf[len - (i + 1)] = buf[i];
	}

	return len;
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
				for(uint8 j = 0; j != segmentLen; j++){
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
				for(uint8 j = 0; j != segmentLen; j++){
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

void fprintf(int32 fd, const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	vprintf(PRINT_SCREEN, fd, 0, 0, fmt, args);
	va_end(args);
}

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(PRINT_SCREEN, 0, 0, 0, fmt, args);
	va_end(args);
}

int snprintf(char *s, unsigned int n, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int result = vprintf(PRINT_BUFFER, 0, s, n, fmt, args);
	va_end(args);
	return result;
}

FILE *fopen(const char *restrict filename, const char *restrict mode) {
    int omode = O_RDWR; //HACK
    if(mode == 0){
        omode = O_RDONLY;
    }
    int fd = open(filename, omode);
    FILE *result = malloc(sizeof(FILE));
    result->fd = fd;

    return result;
}
