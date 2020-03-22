#ifndef JOS_INC_ASSERT_H
#define JOS_INC_ASSERT_H


#define _str(x) #x
#define _tostr(x) _str(x)
#define _assert_occurs " [" __FILE__ ":" _tostr(__LINE__) "] "
#define assert(x) \
        do { if (!(x)) panic("assertion failed" _assert_occurs #x); } while (0)


#endif /* !JOS_INC_ASSERT_H */
