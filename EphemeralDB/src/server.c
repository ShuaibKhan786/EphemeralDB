#include "utils.h"

int global = 1;

struct addrinfo *setUpSai(){
    struct addrinfo hints;

    memset(&hints,0,sizeof hints);

    hints.ai_flags = AI_PASSIVE;    
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = 0;

    struct addrinfo *res;

    int gai_stat = getaddrinfo(NULL,
                            PORT,
                            &hints,
                            &res);
    if(gai_stat > 0){
        printf("%s\n",gai_strerror(gai_stat));
        return NULL;
    }

    return res;
}


 bool setNonblocking(const int sock_fd){
    int flags = fcntl(sock_fd,
                    F_GETFL,
                    0);
    if(flags == -1){
        return false;
    }

    flags |= O_NONBLOCK;

    int fcntl_stat = fcntl(sock_fd,
                        F_SETFL,
                        flags);
    if(fcntl_stat == -1){
        return false;
    }
    
    return true;
 }


int initServer(){
    struct addrinfo *sai = setUpSai();
    if(sai == NULL){
        return -1;
    }
    
    int sock_fd = -1;
    struct addrinfo *temp = sai;

    while(temp != NULL){
        sock_fd = socket(temp->ai_family,
                    temp->ai_socktype,
                    temp->ai_protocol);
        if(sock_fd == -1){
            temp = temp->ai_next;
            continue;
        }
        
        int yes = 1; 
        int sso_stat = setsockopt(sock_fd,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &yes,
                        sizeof(int));
        if(sso_stat < 0){
            close(sock_fd);
            temp = temp->ai_next;
            continue;
        }  
            
        int bind_stat = bind(sock_fd,
                         temp->ai_addr,
                                        temp->ai_addrlen);
        if(bind_stat < 0){
            perror("BIND ");
            close(sock_fd);
            temp = temp->ai_next;
            continue;
        }
        break;
    }
    if(temp == NULL){
        goto label_fai; 
    }
 
    bool setNonblocking(const int);
    if(!setNonblocking(sock_fd)){
        printf("Failed to set the socket to non-blocking socket\n");
        close(sock_fd);
        sock_fd = -1;
        goto label_fai;
    }

    int listen_stat;

    listen_stat = listen(sock_fd,
                        BACKLOG);
    if(listen_stat < 0){
        perror("LISTEN ");
        close(sock_fd);
        sock_fd = -1;
        printf("Failed to init the server\n");
        goto label_fai;
    }
    printf("Successfully init the server...\n\n");
    
label_fai:
    freeaddrinfo(sai);
    return sock_fd;
}

bool evLp(int listen_sockfd){
    size_t nfds = 0;
    size_t pfd_size = INITIAL_PFD_SIZE;

    gb.pfds = (struct pollfd*)calloc(sizeof(struct pollfd),
                                            pfd_size);
    if(gb.pfds == NULL){
        return false;
    }

    gb.pfds[0].fd = listen_sockfd;
    gb.pfds[0].events = POLLIN;
    nfds = 1;

    for(;;){
        int poll_count = poll(gb.pfds,
                            nfds,
                            -1);
        if(poll_count < 0){
            continue;
        }

        for(size_t fd = 0 ; fd < nfds ; fd++){
            if(gb.pfds[fd].revents & POLLIN){
                if(gb.pfds[fd].fd == listen_sockfd){
                    /*
                     * process of accepting
                     * new client are handle 
                     * by the main thread
                     */
                    struct sockaddr_storage client_addr;
                    socklen_t client_addrlen = sizeof client_addr;
    
                    int client_sockfd = accept(listen_sockfd,
                                            (struct sockaddr*)&client_addr,
                                            &client_addrlen);
                    if(client_sockfd < 0){
                        continue;
                    }
                    

                    bool setNonblocking(const int);
                    if(!setNonblocking(client_sockfd)){
                        shutdown(client_sockfd,SHUT_RDWR);
                        continue;
                    }
                  
                    pthread_mutex_lock(&(gb.tp.mu));
                    printf("A client with %d is connected\n",client_sockfd);
                    bool addToPfds(const int,size_t*,size_t*);
                    if(!addToPfds(client_sockfd,&nfds,&pfd_size)){
                        //do something if first doesnt work
                        break;
                    }
                    pthread_mutex_unlock(&(gb.tp.mu));
                    continue;
                }else{
                   /*
                    * process of identifying 
                    * the operation are handle
                    * by main thread
                    */
                    DataTransmission data;
                    
                    pthread_mutex_lock(&(gb.tp.mu));
                    data.client_sockfd = gb.pfds[fd].fd;
                    data.nfds = &nfds;
                    pthread_mutex_unlock(&(gb.tp.mu));
                    data.index = fd ;

                    void handleDataTransmission(DataTransmission);
                    handleDataTransmission(data);

                    continue;
                }
            }
        }
    }
}

void handleDataTransmission(DataTransmission data){
    /*
     * Identify the operation SET GET DEL UDP
     */
    ssize_t byte_received = recv(data.client_sockfd,
                            gb.common_buffer.buffer,
                            (size_t)sizeof(uint8_t),
                            0);

    if(byte_received == 0){
       
        close(data.client_sockfd);

        pthread_mutex_lock(&(gb.tp.mu));
        void remFromPfds(const size_t, size_t*);
        remFromPfds(data.index,data.nfds);
        pthread_mutex_unlock(&(gb.tp.mu));
        return;
    }


    if(byte_received == -1){
        sendErrRes(data.client_sockfd, (uint8_t)UDEFINED_OPERATION);
        close(data.client_sockfd);

        pthread_mutex_lock(&(gb.tp.mu));
        void remFromPfds(const size_t, size_t*);
        remFromPfds(data.index,data.nfds);
        pthread_mutex_unlock(&(gb.tp.mu));
        
    }

    int op = (int)((uint8_t*)gb.common_buffer.buffer)[0];

    switch(op){
        case GET:
            parseGetAndDel(data.client_sockfd,(uint8_t*)gb.common_buffer.buffer);
            break;
        case SET:
            parseSetAndUpd(data.client_sockfd,(uint8_t*)gb.common_buffer.buffer);
            break;
        case UPD:
            parseSetAndUpd(data.client_sockfd,(uint8_t*)gb.common_buffer.buffer);
            break;
        case DEL:
            parseGetAndDel(data.client_sockfd,(uint8_t*)gb.common_buffer.buffer);
            break;
        case EXIST:
            parseGetAndDel(data.client_sockfd,(uint8_t*)gb.common_buffer.buffer);
            break;
        default:
            {
                sendErrRes(data.client_sockfd, (uint8_t)UDEFINED_OPERATION);       
                close(data.client_sockfd);
                
                pthread_mutex_lock(&(gb.tp.mu));
                void remFromPfds(const size_t, size_t*);
                remFromPfds(data.index,data.nfds);
                pthread_mutex_unlock(&(gb.tp.mu));
                    
            }
            break;
    }
}


bool addToPfds(const int new_sockfd,size_t *nfds,size_t *pfd_size){
    bool success = true;

    if(*nfds >= (int)((*pfd_size) * POLLFDS_THRESHOLD)){
        *pfd_size *= PFD_DOUBLE_SIZE;
        struct pollfd *temp = (struct pollfd*) realloc(gb.pfds,
                                                sizeof(struct pollfd) * (*pfd_size));
        assert(temp != NULL);
        if(temp == NULL){
            success = false;
            goto label_addFd;
        } 
        gb.pfds = temp;
    }

label_addFd:
    gb.pfds[*nfds].fd = new_sockfd;
    gb.pfds[*nfds].events = POLLIN;

    (*nfds)++;
    return success;
}


void remFromPfds(const size_t index, size_t *nfds) {
    if (*nfds == 0 || index >= *nfds) {
        return;
    }
    
    gb.pfds[index] = gb.pfds[*nfds - 1];
   
    memset(&gb.pfds[*nfds - 1], 0, sizeof(struct pollfd));

    (*nfds)--;
}

void pfdsCleanup() {
    if (gb.pfds != NULL) {
        free(gb.pfds);
        gb.pfds = NULL; 
    }
}


