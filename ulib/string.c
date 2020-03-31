#include "types.h"
#include "x86.h"

int memcmp(const void* v1, const void* v2, uint n) {
    const uchar* s1, * s2;

    s1 = v1;
    s2 = v2;
    while (n-- > 0) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++, s2++;
    }

    return 0;
}


void* memset(void* dst, int c, uint n) {
    if ((uintp)dst % 4 == 0 && n % 4 == 0) {
        c &= 0xFF;
        stosl(dst, (c << 24) | (c << 16) | (c << 8) | c, n / 4);
    } else
        stosb(dst, c, n);
    return dst;
}

void* memmove(void* dst, const void* src, uint n){
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

int strlen(const char* s) {
    int n;

    for (n = 0; s[n]; n++)
        ;
    return n;
}

char* strcpy(char *s, char *t) {
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

char* strcat_s(char *dest, char *right, int max_len) {
    int writing = -1;
    for(int i = 0; i !=max_len; i++){
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

char* strncpy(char* s, const char* t, int n) {
    char* os;

    os = s;
    while (n-- > 0 && (*s++ = *t++) != 0)
        ;
    while (n-- > 0)
        *s++ = 0;
    return os;
}

int strncmp(const char* p, const char* q, uint n) {
    while (n > 0 && *p && *p == *q)
        n--, p++, q++;
    if (n == 0)
        return 0;
    return (uchar) * p - (uchar) * q;
}

int atoi(const char *s) {
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

char* strchr(const char *s, char c) {
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char* strstr(const char *str1, char *str2) {
    int32 len = strlen(str2);
    const char sentinel = '\0';

    while (*str1 != sentinel) {
        if (strncmp(str1, str2, len) == 0){
            return (char *) str1;
        }
        str1++;
    }

    return 0;
}
