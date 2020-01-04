#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"
#include "future.h"

void f(void* arg, size_t argsz) {
    sleep(4);
    printf("testy\n");
}

void f1(void* arg, size_t argsz) {
    printf("testy1\n");
}

void f2(void* arg, size_t argsz) {
    printf("testy2\n");
}

void f3(void* arg, size_t argsz) {
    sleep(4);
    printf("testy3\n");
}

void* double_(void* arg, size_t argsz, size_t* ressz) {
    //sleep(1);
    *ressz = 18;
    int* a=  malloc(sizeof(int));
    *a = (*(int*)(arg))*2;
    return a;
}

int main() {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 5);
/*
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
    defer(pool, r2);
*/
    callable_t callable;
    int a = 5;
    callable.arg = &a;
    callable.argsz = 1;
    callable.function = double_;

    future_t* future = malloc(sizeof(future_t));
    async(pool, future, callable);
    printf("%d\n", *(int*)await(future));
    printf("%d\n", (int)(future->ressz));

    future_t* future1 = malloc(sizeof(future_t));

    map(pool, future1, future, double_);
    printf("%d\n", *(int*)await(future1));
    printf("%d\n", (int)(future1->ressz));
    printf("%d\n", *(int*)await(future1));
    printf("%d\n", (int)(future1->ressz));


    //sleep(3);
    thread_pool_destroy(pool);

    free(pool);
    free(future->res);
    free(future);



    return 0;
}