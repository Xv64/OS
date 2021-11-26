#include "types.h"
#include "user.h"
#include "fcntl.h"

// same parameters as MMIX by Knuth
// see: https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
#define m 0x8000000000000000
#define a 6364136223846793005
#define c 1442695040888963407

void main(void){
	if (!isblessed()) {
		fprintf(stdout, "Not blessed, exiting\n");
		procexit();
	}

	mkvdev("/dev/random");
	int fd = open("/dev/random", O_WRONLY);
	if(fd <= 0) {
		fprintf(stderr, "Error opening /dev/random\n");
		procexit();
	}

	uint64 last = 1;
	uint64 seed = 1982;

	uint8 delay = 30;
	while(1) {
		uint64 rnd = (last % seed) + 1;
		last = (a * last + c) % m;

		char str[8];
		str[0] = (char)rnd;
		str[1] = (char)(rnd >> 8);
		str[2] = (char)(rnd >> 16);
		str[3] = (char)(rnd >> 24);
		str[4] = (char)(rnd >> 32);
		str[5] = (char)(rnd >> 40);
		str[6] = (char)(rnd >> 48);
		str[7] = (char)(rnd >> 56);

		write(fd, &str, 8);
		if(delay > 0){
			sleep(delay--);
		}
	}
}
