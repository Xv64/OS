#include "syscalls.h"

// ulib.c
#define TIOCGWINSZ 0x100
int   stat(char*, struct stat*);
char* gets(char *buf, int max);
int32 ioctl(int32 fd, uint64 cmd, ...);


// printf.c
#define stdout 1
#define stderr 2

void  fprintf(int32, const char*, ...);


// umalloc.c
void* malloc(uint);
void  free(void*);

uint32 getppid(void);
