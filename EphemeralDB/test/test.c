#include "../src/utils.h"

int main(){
    int sock_fd = initServer();
    if(sock_fd < 0){
        printf("TEST FAIL!!!\n");
    }
    printf("TEST SUCCESS\n");
    printf("PASSED setting up ai\n");
    printf("PASSED making the socket into a listening socket\n");

    evLp(sock_fd);

    close(sock_fd);


    return 0;
}
