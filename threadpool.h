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
    size_t onCondition;     //wątki na c1
    pthread_cond_t c2;      //inicjalizacja w init
    element* queue;
    element* last;
    pthread_attr_t attr;
    int still_work;   //1 - ture, 0 - false
    int poolDelete;   //1 - ture, 0 - false

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
        toDo = pop(pool);
        while (pool->still_work && toDo == NULL) {
            pool->onCondition++;
            if (pool->poolDelete && pool->onCondition == pool->size) {
                if ((err = pthread_cond_signal(&(pool->c2))) != 0) {
                    syserr(err, "Signal error.");
                }
            }
            if ((err = pthread_cond_wait(&(pool->c1), &(pool->m1))) != 0) {
                syserr(err, "Error in wait");
            }
            pool->onCondition--;
            toDo = pop(pool);
        }
        if ((err = pthread_mutex_unlock(&pool->m1)) != 0)
            syserr (err, "unlock failed");
        if (toDo) {
            printf("mam robote\n");
            (toDo->function)(toDo->arg, toDo->argsz);
            free(toDo);
        }
    }
    return 0;
}

int thread_pool_init(thread_pool_t *pool, size_t pool_size) {
    pool->size = pool_size;
    pool->th = malloc(sizeof(pthread_t)*pool_size);
    pool->still_work = 1;
    pool->poolDelete = 0;
    pool->onCondition = 0;
    pool->last = NULL;
    pool->queue = NULL;
    int err;

    if ((err = pthread_mutex_init(&pool->m1, 0) != 0)) {
        syserr(err, "mutex init failed");
    }
    if ((err = pthread_cond_init(&pool->c1, 0)) != 0) {
        syserr(err, "cond1 init failed");
    }
    if ((err = pthread_cond_init(&pool->c2, 0)) != 0) {
        syserr(err, "cond2 init failed");
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
    pool->poolDelete = true;

    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock");
    }
    while (pool->onCondition < pool->size) {

        if ((err = pthread_cond_wait(&(pool->c2), &(pool->m1))) != 0) {
            syserr(err, "Wait error.");
        }
    }

    pool->still_work = false;
    void* res;
    for (size_t i = 0; i < pool->size; i++) {
        if ((err = pthread_cond_signal(&pool->c1)) != 0) {
            syserr(err, "signal failed");
        }
    }

    if ((err = pthread_mutex_unlock(&(pool->m1))) != 0) {
        syserr(err, "Error in unlock");
    }

    for (size_t i = 0; i < pool->size; i++) {
        if ((err = pthread_join((pool->th)[i], &res)) != 0)
            syserr(err, "join failed");
    }

    free(pool->th);
    while (pool->queue) {
        element* n = (pool->queue)->next;
        free(pool->queue);
        pool->queue = n;
    }
    if ((err = pthread_mutex_destroy(&pool->m1) != 0)) {
        syserr(err, "mutex destroy failed");
    }
    if ((err = pthread_cond_destroy(&pool->c1)) != 0) {
        syserr(err, "cond1 destroy failed");
    }
    if ((err = pthread_cond_destroy(&pool->c2)) != 0) {
        syserr(err, "cond2 destroy failed");
    }
    if ((err = pthread_attr_destroy(&pool->attr)) != 0)
        syserr(err, "attr destroy failed");
}

int defer(thread_pool_t *pool, runnable_t runnable) {
    if(pool->poolDelete)
        return ERR;
    int err;
    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock.");
    }
    push(pool, runnable.function, runnable.arg, runnable.argsz);
    if ((err = pthread_cond_signal(&(pool->c1))) != 0) {
        syserr(err, "Signal error.");
    }
    if ((err = pthread_mutex_unlock(&pool->m1)) != 0)
        syserr (err, "unlock failed");
    return SUCC;
};

#endif //THREAD_POOL_THREADPOOL_H
