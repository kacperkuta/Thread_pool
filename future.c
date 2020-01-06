#define ERR -1
#define SUCC 0

#include "future.h"

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
