//
// Created by baka475 on 04.01.20.
//

#ifndef THREAD_POOL_FUTURE_H
#define THREAD_POOL_FUTURE_H
typedef struct callable {
    void *(*function)(void *, size_t, size_t *);
    void *arg;
    size_t argsz;
} callable_t;

typedef struct future {
} future_t;

int async(thread_pool_t *pool, future_t *future, callable_t callable);

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

void *await(future_t *future);
#endif //THREAD_POOL_FUTURE_H
