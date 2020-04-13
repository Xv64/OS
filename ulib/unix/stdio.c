#include "syscalls.h"
#include "unix/stdio.h"

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
