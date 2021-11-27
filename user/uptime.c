#include "types.h"
#include "user.h"

int main(int argc, char **argv) {
    uint64 t = ticks();
    fprintf(stdout, "Up %d ticks\n", t);
	procexit();
}
