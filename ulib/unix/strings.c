#include "unix/string.h"

char *index(const char *s, int c) {
	//"The index( ) function shall be equivalent to strchr( )." - POSIX Base Definitions, Issue 6
	return strchr(s, c);
}
