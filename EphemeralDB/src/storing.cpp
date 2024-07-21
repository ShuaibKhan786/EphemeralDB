#include "utils.h"
#include <string>

static bool compareData(const uint8_t* le_k, uint16_t le_ks, const uint8_t* re_k, uint16_t re_ks) {
    if (le_ks != re_ks) {
        return false;
    }
    for (size_t i = 0; i < le_ks; i++) {
        if (le_k[i] != re_k[i]) {
            return false;
        }
    }
    return true;
}


static bool entry_eq(HNode *lhs, HNode *rhs) {
    struct Entry *le = container_of(lhs, struct Entry, node);
    struct Entry *re = container_of(rhs, struct Entry, node);
    return compareData((uint8_t*)le->key,le->key_size,(uint8_t*)re->key,re->key_size);
}

static uint64_t str_hash(const uint8_t *data, size_t len) {
    uint32_t h = 0x811C9DC5;
    for (size_t i = 0; i < len; i++) {
        h = (h + data[i]) * 0x01000193;
    }
    return h;
}

void existKey(ParseData data,int client_sockfd) {
    Entry key_entry;
    key_entry.key = data.key;
    key_entry.key_size = data.key_size;
    key_entry.node.hcode = str_hash((uint8_t*)data.key, data.key_size);

    pthread_mutex_lock(&(gb.tp.mu));
    HNode *node = hm_lookup(&g_data_db, &key_entry.node, &entry_eq);
    pthread_mutex_unlock(&(gb.tp.mu));

    if (node) {
        sendErrRes(client_sockfd, KEY_FOUND);
    }else {
        sendErrRes(client_sockfd, KEY_NOT_FOUND);
    }
}

void setData(ParseData data, int client_sockfd) {
    if (data.key == NULL || data.value == NULL) {
        return;
    }

    Entry key_entry;
    key_entry.key = data.key;
    key_entry.key_size = data.key_size;
    key_entry.node.hcode = str_hash((uint8_t*)data.key, data.key_size);

    pthread_mutex_lock(&(gb.tp.mu));

    HNode *node = hm_lookup(&g_data_db, &key_entry.node, &entry_eq);

    if (node) {
        Entry *old_entry = container_of(node, Entry, node);
        old_entry->val_size = data.value_size;
        free(old_entry->val);
        old_entry->val = data.value;
        free(data.key);
    } else {
        Entry *entry = new Entry();
        entry->key = data.key;
        entry->key_size = data.key_size;
        entry->val_size = data.value_size;
        entry->val = data.value;

        entry->node.hcode = str_hash((uint8_t*)data.key, data.key_size);

        hm_insert(&g_data_db, &entry->node);
    }

    sendSuccRes(client_sockfd);
    pthread_mutex_unlock(&(gb.tp.mu));
}


void getData(void *arg){
    TpFnArg *tpFnArg = (TpFnArg*)arg;

    Entry key_entry;
    key_entry.key = tpFnArg->key;
    key_entry.key_size = tpFnArg->key_size;
    key_entry.node.hcode = str_hash((uint8_t*)tpFnArg->key, tpFnArg->key_size);
    
    pthread_mutex_lock(&(gb.tp.mu));
    HNode *node = hm_lookup(&g_data_db, &key_entry.node, &entry_eq);

    if (node){
        Entry *entry = container_of(node, Entry, node);
        sendSuccRes(tpFnArg->client_sockfd);
        sendData(tpFnArg->client_sockfd,entry->val,entry->val_size);        
    }else {
        sendErrRes(tpFnArg->client_sockfd, KEY_NOT_FOUND);
    }
    pthread_mutex_unlock(&(gb.tp.mu));
    free(tpFnArg->key);
    free(tpFnArg);
}


void deleteData(ParseData data,int client_sockfd){
    Entry key_entry;
    key_entry.key = data.key;
    key_entry.key_size = data.key_size;
    key_entry.node.hcode = str_hash((uint8_t*)data.key, data.key_size);

    pthread_mutex_lock(&(gb.tp.mu));
    HNode *node = hm_pop(&g_data_db, &key_entry.node, &entry_eq);

    if (node){
        Entry *entry = container_of(node, Entry, node);
        free(entry->key);
        free(entry->val);
        delete entry;
        sendSuccRes(client_sockfd);
    }else {
        sendErrRes(client_sockfd, KEY_NOT_FOUND);
    }
    pthread_mutex_unlock(&(gb.tp.mu));
    free(data.key);
}

void updateData(ParseData data, int client_sockfd) {
    if (data.key == NULL || data.value == NULL) {
        return;
    }

    Entry key_entry;
    key_entry.key = data.key;
    key_entry.key_size = data.key_size;
    key_entry.node.hcode = str_hash((uint8_t*)data.key, data.key_size);

    pthread_mutex_lock(&(gb.tp.mu));
    HNode *node = hm_lookup(&g_data_db, &key_entry.node, &entry_eq);

    if (node) {
        Entry *entry = container_of(node, Entry, node);
        entry->val_size = data.value_size;
        free(entry->val);
        entry->val = data.value;
        sendSuccRes(client_sockfd);
    } else {
        sendErrRes(client_sockfd, KEY_NOT_FOUND); //5
    }
    pthread_mutex_unlock(&(gb.tp.mu));
    
    free(data.key);
}




