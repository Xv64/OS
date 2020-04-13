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

int   fgetc(FILE *stream);
FILE *fopen(const char *restrict filename, const char *restrict mode);
void  fprintf(int fd, const char *fmt, ...);
void  printf(const char *fmt, ...);
int   snprintf(char *s, size_t n, const char *fmt, ...);
