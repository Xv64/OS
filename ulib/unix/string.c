#include "unix/string.h"
#include "unix/stddef.h"
#include "syscalls.h"
#include "x86.h"

int32_t atoi(const char *s) {
  int32_t n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

char* gets(char *buf, int32_t max) {
  int32_t i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int32_t memcmp(const void* v1, const void* v2, uint32_t n) {
    const unsigned char* s1, * s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}

void* memmove(void* dst, const void* src, uint32_t n){
    const char* s;
    char* d;

    s = src;
    d = dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    } else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

void* memset(void* dst, int32_t c, uint32_t n) {
    if ((size_t)dst % 4 == 0 && n % 4 == 0) {
        c &= 0xFF;
        stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
    } else
        stosb(dst, c, n);
    return dst;
}

// Like strncpy but guaranteed to NUL-terminate.
char* safestrcpy(char* s, const char* t, int32_t n) {
    char* os;

    os = s;
    if (n <= 0)
        return os;
    while (--n > 0 && (*s++ = *t++) != 0)
        ;
    *s = 0;
    return os;
}

char* strcat_s(char *dest, char *right, int32_t max_len) {
    int32_t writing = -1;
    for(int32_t i = 0; i !=max_len; i++){
        if(writing < 0 && dest[i] == 0){
            writing = i;
        }
        if(writing >= 0){
            dest[i] = right[i - writing];
            if(dest[i] == 0){
                break;
            }
        }
    }
    return dest;
}

char* strchr(const char *s, char c) {
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char* strcpy(char *s, char *t) {
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int32_t strlen(const char* s) {
    int32_t n;

    for (n = 0; s[n]; n++)
        ;
    return n;
}

int32_t strcmp(const char *s1, const char *s2) {
    return strncmp(s1, s2, (uint32_t)(-1));
}

char* strerror(int errnum) {
    //POSIX Base Definitions, Issue 6 - page 1422
    return ""; //HACK
}

int32_t strncmp(const char* p, const char* q, uint32_t n) {
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (unsigned char) * p - (unsigned char) * q;
}

char* strncpy(char* s, const char* t, int32_t n) {
    char* os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;
    while (n-- > 0)
        *s++ = 0;
    return os;
}

char* strstr(const char *str1, char *str2) {
    int32_t len = strlen(str2);
    const char sentinel = '\0';

    while (*str1 != sentinel) {
        if (strncmp(str1, str2, len) == 0){
            return (char *) str1;
        }
        str1++;
    }

    return 0;
}
