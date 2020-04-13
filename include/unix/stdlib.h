//stdlib.h - POSIX Base Definitions, Issue 6 - page 327

#include "stddef.h"

void abort(void);
long atol(const char *str);
void *malloc(size_t size);

//our syscall exit() takes no args, POSIX expects an int status
#define exit(status) (exit())
