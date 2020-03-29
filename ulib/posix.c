#include "types.h"
#include "user.h"
#include "poll.h"

int32 poll(struct pollfd fds[], nfds_t nfds, int32 timeout) {
    //See: UNIX Systems Programming for SVR4 (1e), page 149
    //   & POSIX Base Definitions, Issue 6, page 858

    nfds_t selected = 0; //count of fds with non-zero revents on completion
    for(nfds_t i = 0; i < nfds; i++){
        if(fds[i].fd < 0){
            continue;
        }

        fds[i].revents = 0; //clear revents

        //check events, if no requested events are set
        uint16 events = fds[i].events;
        if((events & POLLIN) == POLLIN){

        }
        sleep(timeout);

        //set POLLHUP, POLLERR, & POLLNVAL in events, even if not requested

        //TODO: update

        if(fds[i].revents != 0){
            selected++;
        }
    }
    return selected;
}
