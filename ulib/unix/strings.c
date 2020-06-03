#include "string.h"

char *index(const char *s, int c) {
    //"The index( ) function shall be equivalent to strchr( )." - POSIX Base Definitions, Issue 6
    return strchar(s, c);
}
