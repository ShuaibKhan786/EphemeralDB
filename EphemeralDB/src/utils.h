#ifndef EPHEMERAL_DB_H
#define EPHEMERAL_DB_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <cstddef>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>

#include <signal.h>

typedef enum {
    FAIL = 0,
    SUCC = 1,
    UDEFINED_OPERATION = 2,
    RESEND = 3,
    DATA_LIMIT = 4,
    KEY_NOT_FOUND = 5,
    KEY_FOUND = 6,
}Err;

/*
 * server.c file
 */
#define PORT "5040"
#define BACKLOG 10
#define POLLFDS_THRESHOLD 0.80
#define INITIAL_PFD_SIZE 10
#define PFD_DOUBLE_SIZE 2

int initServer();
bool evLp(int listen_sockfd);
void pfdsCleanup();

typedef struct {
    int client_sockfd;
    size_t index;
    size_t *nfds;
}DataTransmission;

typedef enum {
    GET = 1,
    SET,
    UPD,
    DEL,
    EXIST,
} Operation;

/*
 * sighan.c file
 */
bool signalHandler(int signum);


/*
 * buffers.c file
 */
#define INITIAL_BUFFER_SIZE 1024                                                
#define BUFFER_THRESHOLD 0.80
typedef struct {
    void *buffer;
    uint32_t buffer_size;
} Buffer;

bool initCommonBuffer(Buffer *buff);
void commonBufferCleanup();
bool checkCommonBufferResize(uint32_t check_against);
void* resizeCommonBuffer(uint32_t resize_value, uint32_t buff_wr);


/*
 * threadpool.cpp file
 */
#include <pthread.h>
#include <queue>
#include <vector>


typedef struct {
    void (*f) (void*);
    void *arg;
}Task;


typedef struct {
    std::vector<pthread_t> threads;
    std::deque<Task> task_queue;
    pthread_mutex_t mu;
    pthread_cond_t condv;
    bool state;
}Threadpool;


bool tpInit(Threadpool *tp,uint8_t num_of_threads);
void addTaskToQueue(Task task,Threadpool *tp);
void tpDestroy(Threadpool *tp);


#define DATA_LIMIT_GET_AND_DEL 1 + 4 + 2 + 1024 * 4
#define DATA_LIMIT_SET_AND_UPD 1 + 4 + 2 + 1024 * 4 + 4 + 1024 * 1024

void sendErrRes(int client_sockfd , uint8_t err_no);
void sendSuccRes(int client_sockfd);

typedef struct {
    uint16_t key_size;
    uint32_t value_size;
    void *key;
    void *value;
}ParseData;

void parseGetAndDel(int client_sockfd,uint8_t *buffer);
void parseSetAndUpd(int client_sockfd,uint8_t *buffer);
void sendData(int client_sockfd,void *value,uint32_t value_size);

typedef struct {
    int client_sockfd;
    uint16_t key_size;
    void* key;
} TpFnArg;

/*
 * hmap.c file
 */
struct HNode {
    HNode *next = NULL;
    uint64_t hcode = 0;
};

// a simple fixed-sized hashtable
struct HTab {
    HNode **tab = NULL;
    size_t mask = 0;
    size_t size = 0;
};

// the real hashtable interface.
// it uses 2 hashtables for progressive resizing.
#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) );})

struct HMap {
    HTab ht1;   // newer
    HTab ht2;   // older
    size_t resizing_pos = 0;
};

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_insert(HMap *hmap, HNode *node);
HNode *hm_pop(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
size_t hm_size(HMap *hmap);
void hm_destroy(HMap *hmap);

struct Entry {
    struct HNode node;
    uint16_t key_size;
    void* key;
    uint16_t val_size;
    void* val;
};

/*
 * storing.cpp file
 */
void existKey(ParseData data,int client_sockfd);
void setData(ParseData data,int client_sockfd);
void getData(void *arg);
void updateData(ParseData data,int client_sockfd);
void deleteData(ParseData data,int client_sockfd);



/*
 * Global structs
 */
typedef struct {
    int server_sockfd;
    struct pollfd *pfds;
    Threadpool tp;
    Buffer common_buffer;
}Global;

extern Global gb;

extern HMap g_data_db;

#endif //EPHEMERAL_DB_H
