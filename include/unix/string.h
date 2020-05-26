//string.h - POSIX Base Definitions, Issue 6 - page 31
#include "stdint.h"

#define memcpy(dst, src, n) (memmove(dst, src, n))

int32_t atoi(const char *s);
char*   gets(char *buf, int32_t max);
int32_t memcmp(const void* v1, const void* v2, uint32_t n);
void*   memmove(void* dst, const void* src, uint32_t n);
void*   memset(void* dst, int32_t c, uint32_t n);
char*   safestrcpy(char* s, const char* t, int32_t n);
char*   strcat_s(char *dest, char *right, int32_t max_len);
char*   strchr(const char *s, char c);
char*   strcpy(char *s, char *t);
int32_t strlen(const char* s);
int32_t strcmp(const char *s1, const char *s2);
char*   strerror(int errnum);
int32_t strncmp(const char* p, const char* q, uint32_t n);
char*   strncpy(char* s, const char* t, int32_t n);
char*   strstr(const char *s, char *s2);
