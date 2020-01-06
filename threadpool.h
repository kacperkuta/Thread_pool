//
// Created by baka475 on 01.01.20.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
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


//queue of runnables to run
typedef struct queue_element {
    struct queue_element* next;
    runnable_t* my_task;
    int is_future_pair;
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

//pops first ready-to-run runnable form pool queue
runnable_t* pop(thread_pool_t* pool) {
    if (pool->queue) {
        element* el = pool->queue;
        element* prev = NULL;
        runnable_t* to_return = NULL;
        do {
            if (!el->is_future_pair) {
                to_return = el->my_task;
                if (prev) {
                    prev->next = el->next;
                    free(el);
                } else {
                    pool->queue = el->next;
                    free(el);
                }
                break;
            } else {
                double_future_t* future_pair = el->my_task->arg;
                int err;
                if ((err = pthread_mutex_lock(&(future_pair->second->m))) != 0) {
                    syserr(err, "Error in lock");
                }
                if (future_pair->second->ready) {
                    to_return = el->my_task;
                    if (prev) {
                        prev->next = el->next;
                        free(el);
                    } else {
                        pool->queue = el->next;
                        free(el);
                    }
                    if ((err = pthread_mutex_unlock(&(future_pair->second->m))) != 0) {
                        syserr(err, "Error in unlock");
                    }
                    break;
                } else {
                    prev = el;
                    el = el->next;
                    if ((err = pthread_mutex_unlock(&(future_pair->second->m))) != 0) {
                        syserr(err, "Error in unlock");
                    }
                }
            }
        } while (el);
        return to_return;
    }
    return NULL;
}

//pushes runnable into the last place in pools queue
void push(thread_pool_t* pool, void (*function)(void*, size_t), void* arg, size_t argsz, int is_future_pair) {

    runnable_t* toPush = malloc(sizeof(runnable_t));
    toPush->function = function;
    toPush->arg = arg;
    toPush->argsz = argsz;

    element* newElement = malloc(sizeof(element));
    newElement->next = NULL;
    newElement->my_task = toPush;
    newElement->is_future_pair = is_future_pair;

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

//list of existing pools, necessary for signal service
typedef struct pool_list {
    thread_pool_t* thread_pool;
    struct pool_list* next;
} pool_list_t;

pool_list_t* pools = NULL;

//basic work function for thread
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
            (toDo->function)(toDo->arg, toDo->argsz);
            free(toDo);
        }
    }
    return 0;
}

//removes pool from pool list
void remove_from_pools(thread_pool_t *pool) {
    pool_list_t* list = pools;
    if (list->thread_pool == pool) {
        pools = pools->next;
        free(list);
    } else {
        while (pool != list->next->thread_pool) {
            list = list->next;
        }
        pool_list_t* toFree = list->next;
        list->next = list->next->next;
        free(toFree);
    }
}

void thread_pool_destroy(thread_pool_t *pool) {
    remove_from_pools(pool);

    int err;
    pool->poolDelete = true;

    //waiting for all threads to finish
    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock");
    }
    while (pool->onCondition < pool->size && pool->queue) {
        if ((err = pthread_cond_wait(&(pool->c2), &(pool->m1))) != 0) {
            syserr(err, "Wait error.");
        }
    }

    //deleting struct elements
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

//SIGINT service
void catch() {
    pool_list_t* list = pools;
    while(list) {
        thread_pool_destroy(list->thread_pool);
        pool_list_t* next = list->next;
        free(list);
        list = next;
    }
    exit(0);
}

void signal_service() {
    struct sigaction action;
    sigset_t block_mask;

    sigemptyset (&block_mask);
    sigaddset(&block_mask, SIGINT);

    action.sa_handler = catch;
    action.sa_mask = block_mask;
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, 0) == -1)    /*Nowa obługa SIGINT*/
        fprintf(stderr, "sigaction");
}

//adds pool to list of existing pools
void add_to_pools(thread_pool_t *pool) {
    pool_list_t* first = pools;
    pools = malloc(sizeof(pool_list_t));
    pools->thread_pool = pool;
    pools->next = first;
}

int thread_pool_init(thread_pool_t *pool, size_t pool_size)  {
    add_to_pools(pool);
    signal_service();

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

int defer(thread_pool_t *pool, runnable_t runnable) {
    if(pool->poolDelete) {
        return ERR;
    }

    int err;
    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock.");
    }
    push(pool, runnable.function, runnable.arg, runnable.argsz, false);
    if ((err = pthread_cond_signal(&(pool->c1))) != 0) {
        syserr(err, "Signal error.");
    }
    if ((err = pthread_mutex_unlock(&pool->m1)) != 0)
        syserr (err, "unlock failed");
    return SUCC;
}

int defer_future_pair(thread_pool_t *pool, runnable_t runnable) {
    if(pool->poolDelete) {
        return ERR;
    }

    int err;
    if ((err = pthread_mutex_lock(&(pool->m1))) != 0) {
        syserr(err, "Error in lock.");
    }
    push(pool, runnable.function, runnable.arg, runnable.argsz, true);
    if ((err = pthread_cond_signal(&(pool->c1))) != 0) {
        syserr(err, "Signal error.");
    }
    if ((err = pthread_mutex_unlock(&pool->m1)) != 0)
        syserr (err, "unlock failed");
    return SUCC;
}


#endif //THREAD_POOL_THREADPOOL_H
