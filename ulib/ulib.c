#include <stdarg.h>

#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "console.h"

char*
gets(char *buf, int max)
{
  int i, cc;
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

int
stat(char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int32 ioctl(int32 fd, uint64 cmd, ...){
    va_list ap;
    va_start(ap, cmd); //get additional args

    if(cmd == TIOCGWINSZ){
        //this is really the only call we support right now
        struct winsize *winsz = va_arg(ap, struct winsize *);
        winsz->ws_row = 80; //TODO: actually wire this up to console.c to get real values
        winsz->ws_col = 25; //for now, we are ok because this is the only console mode we support :-(
        return 0;
    }
    return -1;
}
