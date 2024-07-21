#include "utils.h"

/*
 * Producer
 */

static void* worker(void *arg){
    Threadpool *tp = (Threadpool*) arg;

    for(;;){

        pthread_mutex_lock(&(tp->mu));

        while(tp->task_queue.empty() && !(tp->state)){
            pthread_cond_wait(&(tp->condv),&(tp->mu));
        }

        if(tp->state){
            pthread_mutex_unlock(&(tp->mu));
            pthread_exit(NULL);
        }

        Task task = tp->task_queue.front();
        tp->task_queue.pop_front();

        pthread_mutex_unlock(&(tp->mu));

        task.f(task.arg);

    }

}

bool tpInit(Threadpool *tp,uint8_t num_of_threads){
    if(num_of_threads < 0){
        return false;
    }

    tp->state = false;

    int mu_init = pthread_mutex_init(&(tp->mu),NULL);
    if(mu_init != 0){
        return false;
    }

    int condv_init = pthread_cond_init(&(tp->condv),NULL);
    if(condv_init != 0){
        return false;
    }

    tp->threads.resize(num_of_threads);

    pthread_mutex_lock(&(tp->mu));
    for(uint8_t i = 0 ; i < num_of_threads ; i++){
        int thread_create = pthread_create(&(tp->threads[i]),
                                        NULL,
                                        worker,
                                        (void*)tp);
        if(thread_create != 0){
            return false;
        }
    }
    pthread_mutex_unlock(&(tp->mu));
    return true;
}

/* 
 *Consumer
 */

void addTaskToQueue(Task task,Threadpool *tp){
    pthread_mutex_lock(&(tp->mu));

    tp->task_queue.push_back(task);

    pthread_cond_signal(&(tp->condv));

    pthread_mutex_unlock(&(tp->mu));

}

void tpDestroy(Threadpool *tp) {
    pthread_mutex_lock(&(tp->mu));
    
    tp->state = true;
    pthread_cond_broadcast(&(tp->condv));
    
    pthread_mutex_unlock(&(tp->mu));

    for (size_t i = 0; i < tp->threads.size(); ++i) {
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&(tp->mu));
    pthread_cond_destroy(&(tp->condv));
   

    tp->threads.clear();
    tp->task_queue.clear();

}


