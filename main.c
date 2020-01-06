#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "future.h"

void f(void* arg, size_t argsz) {
    sleep(2);
    int* tab = (int*) arg;
    for (int i = 0; i < argsz; i++) {
        printf("%d ", tab[i]);
    }
    printf(" testy\n");
}

void* f1(void* arg, size_t argsz, size_t* ss) {
    printf("testy1\n");
    return NULL;
}

void* f2(void* arg, size_t argsz, size_t* ss) {
    sleep(4);
    printf("testy2\n");
    return NULL;
}

void f3(void* arg, size_t argsz) {
    printf("testy3\n");
}

void* double_(void* arg, size_t argsz, size_t* ressz) {
    sleep(2);
    *ressz = 18;
    int* a=  malloc(sizeof(int));
    *a = (*(int*)(arg))*2;
    return a;
}

int main() {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 2);
    printf("%d\n", getpid());
/*
    runnable_t r1;
    r1.function = &f1;
    r1.argsz = 0;
    r1.arg = NULL;
    defer(pool, r1);

    runnable_t r2;
    r2.function = &f2;
    r2.argsz = 0;
    r2.arg = NULL;
    defer(pool, r2);
*/
    runnable_t r3;
    r3.function = &f3;
    r3.argsz = 0;
    r3.arg = NULL;

    callable_t c;
    c.function = &f2;
    c.arg = NULL;
    c.argsz = 0;

    future_t* from = malloc(sizeof(future_t));
    async(pool, from, c);

    future_t* fut = malloc(sizeof(future_t));

    map(pool, fut, from, &f1);

    defer(pool, r3);

    thread_pool_destroy(pool);
    free(fut);
    free(from);
    free(pool);
    return 0;
}