#include <stdarg.h>
#include "syscalls.h"
#include "unix/stdio.h"
#include "unix/fcntl.h"
#include "unix/stdint.h"
#include "unix/string.h"
#include "unix/stdlib.h"


#define PRINT_SCREEN 1
#define PRINT_BUFFER 2

static FILE stdin_const  = {0, 0};
static FILE stdout_const = {1, 0};
static FILE stderr_const = {2, 0};

FILE *_xv64_stdin  = &stdin_const;
FILE *_xv64_stdout = &stdout_const;
FILE *_xv64_stderr = &stderr_const;

int feof(FILE *stream) {
    /*
        The feof( ) function shall test the end-of-file indicator for the stream pointed to by stream.
        -- POSIX Base Definitions, Issue 6 - page 350
    */
    if(stream->readable == -1){
        return EOF;
    }
    return 0;
}

int fgetc(FILE *stream) {
    if(stream->readable == -1){
        return EOF;
    }
    unsigned char buf = EOF;
    stream->readable = read(stream->fd, &buf, sizeof(buf)) == sizeof(buf) ? 1 : -1;
    if(stream->readable == -1){
        return EOF;
    }
    return (int)buf;
}

char *fgets(char *restrict buf, int n, FILE *restrict stream) {
    /*
        The fgets( ) function shall read bytes from stream into the array pointed to by s, until n−1 bytes
        are read, or a <newline> is read and transferred to s, or an end-of-file condition is encountered.
        The string is then terminated with a null byte.
        -- POSIX Base Definitions, Issue 6 - page 368
    */

    for(int i = 0; i != (n - 1); i++){
        int c = fgetc(stream);
        if(c == EOF){
            buf[i] = '\0';
            break;
        }
        buf[i] = (char)c;
        buf[i+1] = '\0'; //this only is safe because we're looping until n-1

        if(c == '\n'){
            //unlike the test for EOF, we want to
            //copy the newline to the output buffer
            //before we conclude.
            break;
        }
    }
    return buf;
}


static int32_t putc(int fd, char c) {
	write(fd, &c, 1);
	return 1;
}

static int8_t printint(int xx, int base, int sgn, char *outbuf) {
	static char digits[] = "0123456789ABCDEF";
	char buf[16];
	int i, neg;
	uint32_t x;

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
	int32_t len = i;

	while(--i >= 0){
		outbuf[len - (i + 1)] = buf[i];
	}

	return len;
}

// This is the core print method, all external functions delegate to this.
static int32_t vprintf(uint8_t mode, int32_t fd, char *obuf, uint32_t maxlen, const char *fmt,  va_list ap){
	char *s;
	int c, i, state;
	int32_t len = 0; //based on mode this will either represent:
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
						obuf[len] = c;
					}
					len++;
				}
			}
		} else if(state == '%') {
			if(c == 'd') {
				char buf[16];
				int8_t segmentLen = printint(va_arg(ap, int), 10, 1, &buf[0]);
				for(uint8_t j = 0; j != segmentLen; j++){
					if(mode == PRINT_SCREEN) {
						len += putc(fd, buf[j]);
					}else {
						if(len < maxlen) {
							obuf[len] = buf[j];
						}
						len++;
					}
				}
			} else if(c == 'x' || c == 'p') {
				char buf[16];
				int8_t segmentLen = printint(va_arg(ap, int), 16, 0, &buf[0]);
				for(uint8_t j = 0; j != segmentLen; j++){
					if(mode == PRINT_SCREEN) {
						len += putc(fd, buf[j]);
					}else {
						if(len < maxlen) {
                            obuf[len] = buf[j];
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
                            obuf[len] = *s;
						}
						len++;
					}
					s++;
				}
			} else if(c == 'c') {
				if(mode == PRINT_SCREEN) {
					len += putc(fd, va_arg(ap, uint32_t));
				}else{
					if(len < maxlen) {
                        obuf[len] = va_arg(ap, uint32_t);
					}
					len++;
				}
			} else if(c == '%') {
				if(mode == PRINT_SCREEN) {
					len += putc(fd, c);
				}else{
					if(len < maxlen) {
						obuf[len] = c;
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
						obuf[len] = '%';
					}
					len++;
					if(len < maxlen) {
						obuf[len] = c;
					}
					len++;
				}
			}
			state = 0;
		}
	}
	if(mode == PRINT_BUFFER) {
		//null terminate our string, but do NOT increment len in the process
		obuf[ len < maxlen ? len : maxlen ] = '\0';
	}
	return len;
}

int fprintf(FILE *stream, const char *fmt, ...){
	va_list args;
	va_start(args, fmt);
	int result = vprintf(PRINT_SCREEN, stream->fd, 0, 0, fmt, args);
	va_end(args);
    return result;
}

void printf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(PRINT_SCREEN, 0, 0, 0, fmt, args);
	va_end(args);
}

int puts(const char *s) {
    printf("%s\n", s);
    return 0;
}

int snprintf(char *s, size_t n, const char *fmt, ...) {
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
    char fname[2048]; //HACK
    int fd = open(strncpy(&fname[0], filename, 2048), omode);
    FILE *result = malloc(sizeof(FILE));
    result->fd = fd;

    return result;
}

long ftell(FILE *stream) {
    //the Xv64 "seek" syscall returns the file offset after seeking,
    //so if we seek to 0 (e.g. our current location)
    //then we will have the current file offset
    int result = seek(stream->fd, 0);
    if(result < 0){
        return -1;
    }
    return result;
}

int fclose(FILE *f) {
    return close(f->fd);
}

int fseek(FILE *stream, long offset, int whence) {
    //POSIX Base Definitions, Issue 6 - page 442
    if(whence == SEEK_CUR){
        int result = seek(stream->fd, offset);
        if(result < 0){
            return -1;
        }
        return 0;
    }
    return -1; //not supported at the moment
}

int vfprintf(FILE *stream, const char *restrict fmt, va_list args) {
    return vprintf(PRINT_SCREEN, stream->fd, 0, 0, fmt, args);
}
