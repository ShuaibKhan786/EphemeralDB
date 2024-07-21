#include "../src/utils.h"

Global gb;
HMap g_data_db;

int main(void){
    
    signalHandler(SIGINT);
    signalHandler(SIGTERM);
    signalHandler(SIGPIPE);

    if(!initCommonBuffer(&(gb.common_buffer))){
        printf("TEST FAILED : common buffer initialization failed\n");
        return -1;
    }


    if(!tpInit(&(gb.tp),4)){
        commonBufferCleanup();
        printf("TEST FAILED : threadpool initialization failed\n");
        return -1;
    }


    gb.server_sockfd = initServer();
    if(gb.server_sockfd == -1){
        printf("TEST FAILED : server initialization failed\n");
        tpDestroy(&(gb.tp));
        commonBufferCleanup();
        return -1;
    }


    if(!evLp(gb.server_sockfd)){
        printf("TEST FAILED : eventloop initialization failed\n");
        tpDestroy(&(gb.tp));
        commonBufferCleanup();
        close(gb.server_sockfd);
    }


    return 0;
}


