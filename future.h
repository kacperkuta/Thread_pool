#ifndef THREAD_POOL_FUTURE_H
#define THREAD_POOL_FUTURE_H

#include "threadpool.h"


typedef struct callable {
    void* (*function)(void* , size_t, size_t*);
    void* arg;
    size_t argsz;
} callable_t;

/* conditional and mutex should be destroyed by library user!!! */
typedef struct future {
    size_t ressz;                   //size fo result
    void* res;                      //result
    callable_t call;                //callable to do
    pthread_cond_t c;               //conditional for await
    pthread_mutex_t m;              //mutex for multi-thread safety
    int ready;                      //is result ready? 1 - true, 0 - false
    thread_pool_t* waiting_pool;    //pool which is waiting for future to be ready
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
