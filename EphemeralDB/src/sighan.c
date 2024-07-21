#include "utils.h"

void handler(int signum) {
    switch(signum) {
        case SIGINT:
        case SIGTERM:
            tpDestroy(&(gb.tp));
            pfdsCleanup();
            commonBufferCleanup();
            hm_destroy(&g_data_db);
            exit(EXIT_SUCCESS);
    }
}

struct sigaction sa;

bool signalHandler(int signum) {
    if(signum == SIGPIPE){
        sa.sa_handler = SIG_IGN;
    }else{
        sa.sa_handler = handler;
    }
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    int sig_stat = sigaction(signum, &sa, NULL);
 
    if (sig_stat > 0) {
        return false;
    }
    return true;
}

