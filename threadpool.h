//
// Created by baka475 on 01.01.20.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "err.h"

#define ERR -1
#define SUCC 0

typedef struct runnable {
    void (*function)(void*, size_t);
    void* arg;
    size_t argsz;
} runnable_t;

//queue of runnables to run
typedef struct queue_element {
    struct queue_element* next;
    runnable_t* myRunnable;
} element;

typedef struct thread_pool {
    size_t size;
    pthread_t* th;
    pthread_mutex_t m1;     //inicjalizacja w init
    pthread_cond_t c1;      //inicjalizacja w init
    element* queue;
    element* last;
    pthread_attr_t attr;
    int still_work;   //1 - ture, 0 - false
} thread_pool_t;

runnable_t* pop(thread_pool_t* pool) {
    if (pool->queue) {
        runnable_t* toReturn = (pool->queue)->myRunnable;
        element* toFree = pool->queue;
        pool->queue = (pool->queue)->next;
        if (!(pool->queue)) {
            pool->queue = NULL;
            pool->last = NULL;
        }
        free(toFree);
        return toReturn;
    } 
    return NULL;
}

void push(thread_pool_t* pool, void (*function)(void*, size_t), void* arg, size_t argsz) {
    runnable_t* toPush = malloc(sizeof(runnable_t));
    toPush->function = function;
    toPush->arg = arg;
    toPush->argsz = argsz;
    element* newElement = malloc(sizeof(element));
    newElement->next = NULL;
    newElement->myRunnable = toPush;
    if (pool->queue) {
        (pool->last)->next = newElement;
        pool->last = newElement;
    } else {
        pool->queue = newElement;
        pool->last = newElement;
    }
    int err;
    if ((err = pthread_cond_signal(&pool->c1)) != 0) {
        syserr(err, "signal failed");
    }
}

void* work(void* pool_pointer) {
    thread_pool_t* pool = pool_pointer;
    int err;
    while (pool->still_work) {
        if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
            syserr(err, "Error in lock");
        }
        runnable_t* toDo = NULL;
        while (pool->still_work && (toDo = pop(pool)) == NULL) {
            if ((err = pthread_cond_wait(&(pool->c1), &(pool->m1))) == 0) {
                syserr(err, "Error in wait");
            }
        }
        if (toDo && pool->still_work)
            (toDo->function)(toDo->arg, toDo->argsz);
    }
    return 0;
}

int thread_pool_init(thread_pool_t *pool, size_t pool_size) {
    pool->size = pool_size;
    pool->th = malloc(sizeof(pthread_t)*pool_size);
    pool->still_work = true;
    pool->last = NULL;
    pool->queue = NULL;
    int err;

    if ((err = pthread_mutex_init(&pool->m1, 0) != 0)) {
        syserr(err, "mutex init failed");
        return ERR;
    } if ((err = pthread_cond_init(&pool->c1, 0)) != 0) {
        syserr(err, "cond init failed");
        return ERR;
    }

    if ((err = pthread_attr_init (&pool->attr)) != 0)
        syserr(err, "attr_init failed");

    for (size_t i = 0; i < pool_size; i++) {
        if ((err = pthread_create(&(pool->th[i]), &pool->attr, &work, pool)) != 0)
            syserr(err, "create failed");
    }
    return SUCC;
}

void thread_pool_destroy(thread_pool_t *pool) {
    int err;
    pool->still_work = false;
    void* res;
    for (size_t i = 0; i < pool->size; i++) {
        if ((err = pthread_cond_signal(&pool->c1)) != 0) {
            syserr(err, "signal failed");
        }
        if ((err = pthread_join((pool->th)[i], &res)) != 0)
            syserr(err, "join failed");
    }

    free(pool->th);
    while(pool->queue) {
        element* n = (pool->queue)->next;
        free(pool->queue);
        pool->queue = n;
    }
    if ((err = pthread_mutex_destroy(&pool->m1) != 0)) {
        syserr(err, "mutex destroy failed");
    }
    if ((err = pthread_cond_destroy(&pool->c1)) != 0) {
        syserr(err, "cond destroy failed");
    }
    if ((err = pthread_attr_destroy(&pool->attr)) != 0)
        syserr(err, "attr destroy failed");
}

int defer(thread_pool_t *pool, runnable_t runnable) {
    int err;
    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock.");
        return ERR;
    }
    push(pool, runnable.function, runnable.arg, runnable.argsz);
    if ((err = pthread_cond_signal(&(pool->c1))) != 0) {
        syserr(err, "Signal error.");
        return ERR;
    }
    return SUCC;
};

#endif //THREAD_POOL_THREADPOOL_H
