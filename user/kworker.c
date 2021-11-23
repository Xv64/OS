#include "types.h"
#include "user.h"

void main(void){
  if (!isblessed()) {
      fprintf(stdout, "Not blessed, exiting\n");
    procexit();
  }
  while(1){
    // fprintf(stdout, "nom\n");
    sleep(40);
  }
}
