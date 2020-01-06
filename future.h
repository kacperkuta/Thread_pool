#ifndef THREAD_POOL_FUTURE_H
#define THREAD_POOL_FUTURE_H

#include "threadpool.h"


typedef struct callable {
    void* (*function)(void* , size_t, size_t*);
    void* arg;
    size_t argsz;
} callable_t;



typedef struct future {
    size_t ressz;
    void* res;
    callable_t call;
    pthread_cond_t c;
    pthread_mutex_t m;
    int ready;
} future_t;

typedef struct double_future {
    future_t* first;
    future_t* second;
} double_future_t;

int async(thread_pool_t *pool, future_t *future, callable_t callable);

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

void *await(future_t *future);


#endif //THREAD_POOL_FUTURE_H
