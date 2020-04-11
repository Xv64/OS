//stdio.h - POSIX Base Definitions, Issue 6 - page 323
#include "stddef.h"


#define stdin  0
#define stdout 1
#define stderr 2

#define	EOF	(-1)

typedef struct {
    int fd;
    int readable;
} FILE;

//implemented in ulib/printf.c:
void fprintf(int fd, const char *fmt, ...);
void printf(const char *fmt, ...);
int  snprintf(char *s, size_t n, const char *fmt, ...);

//implemented in ulib/posix.c
int  fgetc(FILE *stream);
