#include "types.h"
#include "user.h"
#include "fcntl.h"

void main(void){
    if (!isblessed()) {
        fprintf(stdout, "Not blessed, exiting\n");
        procexit();
    }

    mkvdev("/dev/zero");
    int fd = open("/dev/zero", O_WRONLY);
    if(fd <= 0){
        fprintf(stderr, "Error opening /dev/zero\n");
        procexit();
    }
    while(1){
        write(fd, "\0\0\0\0\0\0\0\0\0\0", 10);
        sleep(1);
    }
}
