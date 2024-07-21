#include "utils.h"


bool initCommonBuffer(Buffer *buff){
    buff->buffer  = malloc(INITIAL_BUFFER_SIZE);

    if(buff->buffer == NULL){
        return false;
    }
    buff->buffer_size = INITIAL_BUFFER_SIZE;
    return true;
}

void commonBufferCleanup(){
    if(gb.common_buffer.buffer != NULL ){
        free(gb.common_buffer.buffer);
    }
}

bool checkCommonBufferResize(uint32_t check_against){
    if(check_against >= (gb.common_buffer.buffer_size * BUFFER_THRESHOLD)){
        return true;
    }
    return false;
}

void* resizeCommonBuffer(uint32_t resize_value, uint32_t buff_wr) {
    //making sure
    //it has a consistency of
    //1024 * times
    uint32_t times = (resize_value + INITIAL_BUFFER_SIZE - 1) / INITIAL_BUFFER_SIZE;
    
    uint32_t temp_size = gb.common_buffer.buffer_size * times;
    void* temp_buffer = realloc(gb.common_buffer.buffer, temp_size);
    
    if (temp_buffer == NULL) {
        temp_buffer = malloc(temp_size);
        if (temp_buffer == NULL) {
            return NULL;
        }
        memcpy(temp_buffer, gb.common_buffer.buffer, buff_wr);
    }

    gb.common_buffer.buffer = temp_buffer;
    gb.common_buffer.buffer_size = temp_size;
    
    return temp_buffer;
}


