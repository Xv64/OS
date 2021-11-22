#include "types.h"
#include "user.h"

void main(void){
  uint32 ppid = getppid();
  if (ppid != 1) {
    procexit();
  }
  while(1){
    // fprintf(stdout, "nom\n");
    sleep(40);
  }
}
