//poll.h - POSIX Base Definitions, Issue 6 - page 287

#define POLLIN      0b000000001
#define POLLRDNORM  0b000000010
#define POLLRDBAND  0b000000100
#define POLLPRI     0b000001000
#define POLLOUT     0b000010000
#define POLLWRNORM  POLLOUT    //equivalent to POLLOUT
#define POLLWRBAND  0b000100000
#define POLLERR     0b001000000
#define POLLHUP     0b010000000
#define POLLNVAL    0b100000000

struct pollfd {
    int fd;     // The following descriptor being polled.
    short events; // The input event flags (see below).
    short revents; // The output event flags (see below).
};

typedef uint64 nfds_t;

//defined in posix.c:
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
