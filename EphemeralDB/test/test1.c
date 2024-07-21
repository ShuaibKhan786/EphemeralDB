// test_threadpool.c
#include "../src/utils.h"



Threadpool tp;

void task_function(void *args) {
    int task_number = *((int*)args);
    pthread_mutex_lock(&(tp.mu));
    printf("Task %d is being executed by thread %ld\n", task_number, pthread_self());
    pthread_mutex_unlock(&(tp.mu));
}


int main(void) {

    tpInit(&tp,5);

    int tasks[1000];
    for (int i = 0; i < 1000; ++i) {
        tasks[i] = i + 1;
    }

    for (int i = 0; i < 1000; ++i) {
        Task task;
        task.f = &task_function;
        task.arg = &tasks[i];
        addTaskToQueue(task,&tp);
    }
   
    sleep(4);

    tpDestroy(&tp);

    return 0;
}

