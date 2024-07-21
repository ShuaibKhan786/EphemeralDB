#include "../src/utils.h"

void func(void *args){
    int fd = *(int*)args;

    printf("The file descriptor is %d\n",fd);
}

int main(void){

    int client_sockfd = 10;
    int client_sockfd0 = 0;
    addTask(func,(void*)&client_sockfd);
    addTask(func,(void*)&client_sockfd0);

    void remTask();
    remTask();
    remTask();
    remTask();

    taskCleanup();

    
    return 0;
}
