//assert.h - POSIX Base Definitions, Issue 6 - page 209

#ifdef NDEBUG

#define assert(ignore)((void) 0)

#else

#define _str(x) #x
#define _tostr(x) _str(x)
#define _assert_occurs " [" __FILE__ ":" _tostr(__LINE__) "] "
#define assert(x) \
        do { if (!(x)) { fprintf(stderr, "assertion failed" _assert_occurs #x); abort(); } } while (0)

#endif
