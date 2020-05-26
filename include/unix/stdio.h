//stdio.h - POSIX Base Definitions, Issue 6 - page 323
#include "stddef.h"
#include <stdarg.h>

#define	EOF	(-1)

typedef struct {
    int fd;
    int readable;
} FILE;

extern FILE *_xv64_stdin;
extern FILE *_xv64_stdout;
extern FILE *_xv64_stderr;

#define stdin  _xv64_stdin
#define stdout _xv64_stdout
#define stderr _xv64_stderr

int   feof(FILE *);
int   fgetc(FILE *stream);
char *fgets(char *restrict, int, FILE *restrict);
FILE *fopen(const char *restrict filename, const char *restrict mode);
int   fclose(FILE *);
int   fprintf(FILE *stream, const char *fmt, ...);
void  printf(const char *fmt, ...);
int   snprintf(char *s, size_t n, const char *fmt, ...);
int   vfprintf(FILE *stream, const char *restrict format, va_list ap);
