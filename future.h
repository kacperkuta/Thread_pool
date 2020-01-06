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

void dummy(void* arg, size_t argsz) {
    future_t* future = (future_t*)arg;
    int err;
    void* res;

    res = future->call.function(future->call.arg, future->call.argsz, &future->ressz);
    if ((err = pthread_mutex_lock(&future->m) != 0)) {
        syserr(err, "mutex lock failed");
    }
    future->res = res;
    future->ready = 1;
    if ((err = pthread_cond_signal(&future->c)) != 0) {
        syserr(err, "cond signal failed");
    }

    if ((err = pthread_mutex_unlock(&future->m) != 0)) {
        syserr(err, "mutex unlock failed");
    }
}

void double_dummy(void* arg, size_t argsz) {
    double_future_t* double_future = (double_future_t*)arg;
    double_future->first->call.arg = double_future->second->res;
    double_future->first->call.argsz = double_future->second->ressz;

    dummy(double_future->first, 1);
    free(double_future);
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


int map(thread_pool_t* pool, future_t* future, future_t* from,
        void* (*function)(void*, size_t, size_t*)) {
    int err;
    if ((err = pthread_mutex_init(&future->m, 0) != 0)) {
        syserr(err, "mutex init failed");
    }

    if ((err = pthread_cond_init(&future->c, 0) != 0)) {
        syserr(err, "mutex init failed");
    }

    future->ressz = 0;
    future->ready = 0;
    future->res = NULL;
    callable_t call;
    call.function = function;
    future->call = call;

    double_future_t* double_future = malloc(sizeof(double_future_t));
    double_future->first = future;
    double_future->second = from;

    runnable_t runnable;
    runnable.function = double_dummy;
    runnable.argsz = 1;
    runnable.arg = double_future;

    return defer_future_pair(pool, runnable);
}

#endif //THREAD_POOL_FUTURE_H
