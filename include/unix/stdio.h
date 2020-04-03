//poll.h - POSIX Base Definitions, Issue 6 - page 323
#include "stddef.h"


#define stdin  0
#define stdout 1
#define stderr 2

//implemented in ulib/printf.c:
int snprintf(char *s, size_t n, const char *fmt, ...);
void fprintf(int fd, char *fmt, ...);
void printf(char *fmt, ...);
