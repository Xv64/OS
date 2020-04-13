
//these are all syscalls
void* memset(void* dst, int c, uint n);
void* memmove(void* dst, const void* src, uint n);
char* safestrcpy(char* s, const char* t, int n);
int   strlen(const char* s);
int   strncmp(const char* p, const char* q, uint n);
char* strncpy(char* s, const char* t, int n);
int   memcmp(const void* v1, const void* v2, uint n);


//defined in ulib/string.c
char* strchr(const char *s, char c);
char* strcpy(char *s, char *t);
char* strcat_s(char *dest, char *right, int max_len);
int atoi(const char *s);
char* strstr(const char *str1, char *str2);
