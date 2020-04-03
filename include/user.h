struct stat;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
void reboot(void);

// ulib.c
#define TIOCGWINSZ 0x100
int   stat(char*, struct stat*);
char* gets(char *buf, int max);
int32 ioctl(int32 fd, uint64 cmd, ...);


// printf.c
#define stdout 1
#define stderr 2

void  fprintf(int32, const char*, ...);


// umalloc.c
void* malloc(uint);
void  free(void*);
