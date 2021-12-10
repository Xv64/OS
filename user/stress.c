#include "types.h"
#include "user.h"
#include "fcntl.h"

void stresser();


  void main(void){
      while(1) {
          if(fork() == 0) {
              stresser();
          }
      }
  }

  void stresser() {
      while(malloc(sizeof(uint64)) > 0){}
  }
