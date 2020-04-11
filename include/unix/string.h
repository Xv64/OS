//string.h - POSIX Base Definitions, Issue 6 - page 31

#define memcpy(dst, src, n) (memmove(dst, src, n))

int   atoi(const char *s);
char* gets(char *buf, int max);
int   memcmp(const void* v1, const void* v2, unsigned int n);
void* memmove(void* dst, const void* src, unsigned int n);
void* memset(void* dst, int c, unsigned int n);
char* safestrcpy(char* s, const char* t, int n);
char* strcat_s(char *dest, char *right, int max_len);
char* strchr(const char *s, char c);
char* strcpy(char *s, char *t);
int   strlen(const char* s);
int   strcmp(const char *s1, const char *s2);
int   strncmp(const char* p, const char* q, unsigned int n);
char* strncpy(char* s, const char* t, int n);
char* strstr(const char *s, char *s2);
