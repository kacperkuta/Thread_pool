#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "err.h"
#include <stdbool.h>

typedef struct runnable {
    void (*function)(void*, size_t);
    void* arg;
    size_t argsz;
} runnable_t;

/* queue of runnables to run */
typedef struct queue_element {
    struct queue_element* next;
    runnable_t* my_task;
    int is_future_pair;
} element;


typedef struct thread_pool {
    size_t size;            //pool size
    pthread_t* th;          //array of working threads
    pthread_mutex_t m1;     //mutex for multi-thread safety
    pthread_cond_t c1;      //condition for unemployed threads
    size_t onCondition;     //amount of waiting threads on condition
    pthread_cond_t c2;      //condition for main thread waiting for pool destroying
    element* queue;         //queue of tasks to do
    element* last;          //last element of queue
    pthread_attr_t attr;    //attr for threads
    int still_work;         //1 - ture, 0 - false
    int poolDelete;         //1 - ture, 0 - false

} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

/* pops first ready-to-run runnable form pool queue */
runnable_t* pop(thread_pool_t* pool);

/* pushes runnable into the last place in pools queue */
void push(thread_pool_t* pool, void (*function)(void*, size_t), void* arg, size_t argsz, int is_future_pair);

#endif //THREAD_POOL_THREADPOOL_H
