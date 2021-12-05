#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    char str[120];

    int len = info(&str[0], 120);
    if (len > 0) {
        fprintf(stdout, str);
        fprintf(stdout, "\n");
    }

    procexit();
}
