#include "types.h"
#include "stat.h"
#include "user.h"
#include "string.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 1){
    fprintf(stderr, "usage: kill pid...\n");
    exit();
  }
  for(i=1; i<argc; i++)
    kill(atoi(argv[i]));
  exit();
}
