//stdlib.h - POSIX Base Definitions, Issue 6 - page 327

#include "stddef.h"

void abort(void);
void exit(int status); //syscall
void *malloc(size_t size);
