
#define memset(dst, c, n) (sys_memset(dst, c, n))
#define memmove(dst, src, n) (sys_memmove(dst, src, n))
#define safestrcpy(s, t, n) (sys_safestrcpy(s, t, n))
#define strlen(c) (sys_strlen(c))
#define strncmp(p, q, n) (sys_strncmp(p, q, n))
#define strncpy(s, t, n) (sys_strncpy(s, t, n))
#define memcmp(v1, v2, n) (sys_memcmp(v1, v2, n))

void* sys_memset(void* dst, int c, uint n);
void* sys_memmove(void* dst, const void* src, uint n);
char* sys_safestrcpy(char* s, const char* t, int n);
int   sys_strlen(const char* s);
int   sys_strncmp(const char* p, const char* q, uint n);
char* sys_strncpy(char* s, const char* t, int n);
int   sys_memcmp(const void* v1, const void* v2, uint n);
