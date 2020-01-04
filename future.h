//
// Created by baka475 on 04.01.20.
//

#ifndef THREAD_POOL_FUTURE_H
#define THREAD_POOL_FUTURE_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "err.h"
#include "threadpool.h"

#define ERR -1
#define SUCC 0

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


void dummy(void* arg, size_t argsz) {
    future_t* future = (future_t*)arg;
    int err;
    if ((err = pthread_mutex_lock(&future->m) != 0)) {
        syserr(err, "mutex lock failed");
    }

    future->ready = 0;
    future->res = future->call.function(future->call.arg, future->call.argsz, &future->ressz);
    future->ready = 1;

    if ((err = pthread_cond_signal(&future->c)) != 0) {
        syserr(err, "cond signal failed");
    }

    if ((err = pthread_mutex_unlock(&future->m) != 0)) {
        syserr(err, "mutex unlock failed");
    }
}

int async(thread_pool_t* pool, future_t* future, callable_t callable) {
    int err;
    if ((err = pthread_mutex_init(&future->m, 0) != 0)) {
        syserr(err, "mutex init failed");
    }

    if ((err = pthread_cond_init(&future->c, 0) != 0)) {
        syserr(err, "mutex init failed");
    }

    future->ressz = 0;
    future->call = callable;
    future->res = NULL;
    future->ready = 0;
    runnable_t runnable;
    runnable.function = dummy;
    runnable.argsz = 1;
    runnable.arg = future;

    return defer(pool, runnable);
}

void* await(future_t* future) {
    int err;
    if ((err = pthread_mutex_lock(&future->m) != 0)) {
        syserr(err, "mutex init failed");
    }
    if (future->ready != 1) {
        if ((err = pthread_cond_wait(&future->c, &future->m)) != 0) {
            syserr(err, "cond init failed");
        }
    }
    if ((err = pthread_mutex_unlock(&future->m) != 0)) {
        syserr(err, "mutex init failed");
    }
    return future->res;
}

int map(thread_pool_t* pool, future_t* future, future_t* from,
        void* (*function)(void*, size_t, size_t*)) {
    void* result = await(from);
    callable_t call;
    call.function = function;
    call.argsz = from->ressz;
    call.arg = result;
    return async(pool, future, call);
}

#endif //THREAD_POOL_FUTURE_H
