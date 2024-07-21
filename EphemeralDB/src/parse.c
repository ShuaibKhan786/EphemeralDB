#include "utils.h"

void sendSuccRes(int client_sockfd){
    uint8_t buff = (uint8_t)SUCC;

    ssize_t byte_sent = send(client_sockfd,
                                (void*)&buff,
                                (size_t)sizeof(buff),
                                0);
    if(byte_sent == -1){
        return;
    }
}


void sendErrRes(int client_sockfd ,uint8_t err_no){
    uint8_t buff = 1;

    memset((void*)&buff,err_no,sizeof(uint8_t));

    ssize_t byte_sent = send(client_sockfd,
                                (void*)&buff,
                                (size_t)sizeof(buff),
                                0);
    if(byte_sent == -1){
        if(errno == EPIPE){
            close(client_sockfd);
            return;
        }
        return;
    }
}

void parseGetAndDel(int client_sockfd,uint8_t *buffer){
    if(buffer == NULL){
        return;
    }

    ParseData data = ParseData{0,0,NULL,NULL};

    uint8_t whole_size_flag = 1;
    uint8_t key_size_flag = 1;
    
    uint32_t recv_data_size = sizeof(uint8_t) + sizeof(uint16_t);
    ssize_t recv_tracker = 1;

    do{
        ssize_t bytes_recv = recv(client_sockfd,
                                    buffer+recv_tracker,
                                    recv_data_size - recv_tracker,
                                    0);
        if(bytes_recv == 0){
            return;
        }

        if(bytes_recv == -1){
            sendErrRes(client_sockfd,RESEND);
        }

        recv_tracker += bytes_recv;
        
        //parsing the whole data size
        if(recv_tracker >= (sizeof(uint8_t) + sizeof(uint16_t)) && whole_size_flag){
            memset(&recv_data_size, 0, sizeof(uint16_t));
            memcpy(&recv_data_size, buffer+sizeof(uint8_t), sizeof(uint16_t));
            recv_data_size = ntohs(recv_data_size);
            whole_size_flag = 0;

            //veryfying client doesnt send to the limit amount of data
            if(recv_data_size >= DATA_LIMIT_GET_AND_DEL){
                sendErrRes(client_sockfd,DATA_LIMIT);
            }

            //common buffer resize logic here
            if(checkCommonBufferResize(recv_data_size)){
                void *temp = resizeCommonBuffer(recv_data_size,(uint32_t) sizeof(uint8_t)+sizeof(uint16_t));
                if(temp == NULL){
                    sendErrRes(client_sockfd,RESEND);
                    return;
                }
                buffer = (uint8_t*)temp;
            }
        }

        //parsing the key_size
        if(recv_tracker >= (sizeof(uint8_t)+sizeof(uint16_t)+sizeof(uint16_t)) && key_size_flag){
            memcpy(&(data.key_size), buffer+sizeof(uint8_t)+sizeof(uint16_t), sizeof(uint16_t));
            data.key_size = ntohs(data.key_size);
            key_size_flag = 0;
        }

    }while(recv_tracker < recv_data_size);
    
    //parsing the key

    if(recv_tracker >= (sizeof(uint8_t)+sizeof(uint16_t)+sizeof(uint16_t)+data.key_size) ){
        data.key = malloc(data.key_size);
        
        if(data.key == NULL){
            sendErrRes(client_sockfd,RESEND);
        }

        memcpy(data.key, buffer+sizeof(uint8_t)+sizeof(uint16_t)+sizeof(uint16_t), data.key_size);
    }else{
        sendErrRes(client_sockfd,RESEND);
        return;
    }

    uint8_t op_no;
    memcpy(&op_no, buffer, sizeof(uint8_t));

    if(op_no == (uint8_t)GET){
        TpFnArg* arg = (TpFnArg*)malloc(sizeof(TpFnArg));
        if (arg == NULL) {
            sendErrRes(client_sockfd,RESEND);
            return;
        }
        arg->client_sockfd = client_sockfd;
        arg->key_size = data.key_size;
        arg->key = data.key;

        Task task;
        task.f = &getData;
        task.arg = arg;
        addTaskToQueue(task,&(gb.tp));
    }else if(op_no == (uint8_t)EXIST) {
        existKey(data,client_sockfd);
    }else {
       deleteData(data,client_sockfd);
    }

}


void parseSetAndUpd(int client_sockfd,uint8_t *buffer){

    ParseData data = ParseData{0,0,NULL,NULL};
    
    uint8_t whole_size_flag = 1;
    uint8_t key_size_flag = 1;
    uint8_t key_flag = 1;
    uint8_t value_size_flag = 1;

    uint32_t recv_data_size =  sizeof(uint8_t) + sizeof(uint32_t);
    ssize_t recv_tracker = 1;

    do{
        ssize_t bytes_recv = recv(client_sockfd,
                                    buffer+recv_tracker,
                                    recv_data_size - recv_tracker,
                                    0);
        if(bytes_recv == 0){
            return;
        }
       
        if(bytes_recv == -1){
            sendErrRes(client_sockfd,RESEND);
            return;
        }
        
        recv_tracker += bytes_recv;

        //parsing the whole data size
        if(recv_tracker >= sizeof(uint8_t) + sizeof(uint32_t) && whole_size_flag){
            recv_data_size = 0;
            memcpy(&recv_data_size, buffer+sizeof(uint8_t), sizeof(uint32_t));
            recv_data_size = ntohl(recv_data_size);
            whole_size_flag = 0;

            if(recv_data_size >= DATA_LIMIT_SET_AND_UPD){
                sendErrRes(client_sockfd,DATA_LIMIT);
            }
            
            //common buffer resize logic here
            if(checkCommonBufferResize(recv_data_size)){
                void *temp = resizeCommonBuffer(recv_data_size, (uint32_t) sizeof(uint8_t)+sizeof(uint32_t));
                if(temp == NULL){
                    sendErrRes(client_sockfd,RESEND);
                    return;
                }
                buffer = (uint8_t*)temp;
            }
        }

        //parsing the key_size
        if(recv_tracker >= sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t) && key_size_flag){
            memcpy(&(data.key_size), buffer+sizeof(uint8_t)+sizeof(uint32_t), sizeof(uint16_t));
            data.key_size = ntohs(data.key_size);
            key_size_flag = 0;
        }

        //parsing the key
        if(recv_tracker >= sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t)+data.key_size && key_flag){
            data.key = malloc(data.key_size);
            if(data.key == NULL){
                sendErrRes(client_sockfd,RESEND);
                return;
            }
            memcpy(data.key, buffer+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t), data.key_size);
            key_flag = 0;
        }

        //parsing the value_size
        if(recv_tracker >= sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t)+data.key_size+sizeof(uint32_t) && value_size_flag){
            memcpy(&(data.value_size), buffer+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t)+data.key_size, sizeof(uint32_t));
            data.value_size = ntohl(data.value_size);
            value_size_flag = 0;
        }

    }while(recv_tracker < recv_data_size);

    
    //parsing the value
    if(recv_tracker >= sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t)+(size_t)data.key_size+sizeof(uint32_t)+data.value_size){
        data.value = malloc(data.value_size);
        if(data.value == NULL ){
            free(data.key);
            sendErrRes(client_sockfd,RESEND);
            return;
        }
        memcpy(data.value, buffer+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint16_t)+data.key_size+sizeof(uint32_t), data.value_size);
    }else{
        free(data.key);
        sendErrRes(client_sockfd,RESEND);
    }
    //at this point we can do whatever we want with the data
    
    uint8_t op_no;
    memcpy(&op_no, buffer, sizeof(uint8_t));

    if(op_no == (uint8_t)SET){
        setData(data,client_sockfd);
    }else{
        updateData(data,client_sockfd);
    }
}

void sendData(int client_sockfd,void *value,uint32_t value_size){
    void *actual_data = malloc(sizeof(uint32_t)+value_size);
    if(actual_data == NULL){
        return;
    }

    uint32_t network_value_size = htonl(value_size);

    memcpy(actual_data,&network_value_size,sizeof(uint32_t));
    memcpy((uint8_t*)actual_data+sizeof(uint32_t),value,value_size);

    ssize_t send_tracker = 0;
    
    while(send_tracker < (value_size + sizeof(uint32_t))){
        ssize_t bytes_send = send(client_sockfd,
                                    (uint8_t*)actual_data+send_tracker,
                                    (value_size+sizeof(uint32_t))-send_tracker,
                                    0);
        
        if(bytes_send == -1){
            free(actual_data);
            return;
        }
        send_tracker += bytes_send;
    }
    free(actual_data);
}

