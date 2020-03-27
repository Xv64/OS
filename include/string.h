#ifndef XV64_STRING
#define XV64_STRING

#define memcpy(dst, src, n) (memmove(dst, src, n))
#define strstr(str1, str2)  strchr(str1, str2)

int memcmp(const void* v1, const void* v2, uint n);
void* memset(void* dst, int c, uint n);
void* memmove(void* dst, const void* src, uint n);
int strlen(const char* s);
char* safestrcpy(char* s, const char* t, int n);
char* strncpy(char* s, const char* t, int n);
int strncmp(const char* p, const char* q, uint n);
int atoi(const char *s);
char* strcpy(char *s, char *t);
char* strcat_s(char *dest, char *right, int max_len);
char* gets(char *buf, int max);
char* strchr(const char*, char c);

#endif
