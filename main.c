#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"

void f(void* arg, size_t argsz) {
    printf("testy\n");
}

void f1(void* arg, size_t argsz) {
    printf("testy1\n");
}

void f2(void* arg, size_t argsz) {
    printf("testy2\n");
}

void f3(void* arg, size_t argsz) {
    printf("testy3\n");
}

int main() {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 5);

    runnable_t r;
    r.function = &f;
    r.argsz = 0;
    r.arg = NULL;
    defer(pool, r);

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

    runnable_t r3;
    r3.function = &f3;
    r3.argsz = 0;
    r3.arg = NULL;
    defer(pool, r3);
    
    
    //sleep(3);
    thread_pool_destroy(pool);



    return 0;
}