#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "threadpool.h"
#include "future.h"

void f(void* arg, size_t argsz) {
    sleep(2);
    int* tab = (int*) arg;
    for (int i = 0; i < argsz; i++) {
        printf("%d ", tab[i]);
    }
    printf(" testy\n");
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
    sleep(2);
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
    r.argsz = 4;
    int * a = malloc(50);
    a[0] = 1; a[1] = 2; a[2] = 3; a[3] = 4;
    r.arg = a;
    for (int i = 0; i < 15; i++)
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
    int b = 5;
    callable.arg = &b;
    callable.argsz = 1;
    callable.function = double_;

    future_t* future = malloc(sizeof(future_t));
    async(pool, future, callable);


    future_t* future1 = malloc(sizeof(future_t));
    future_t* future2 = malloc(sizeof(future_t));


    map(pool, future1, future, double_);
    map(pool, future2, future1, double_);
    printf("zlecilem\n");
    printf("%d\n", *(int*)await(future));
    printf("%d\n", (int)(future->ressz));
    printf("%d\n", *(int*)await(future1));
    printf("%d\n", (int)(future1->ressz));
    printf("%d\n", *(int*)await(future2));
    printf("%d\n", (int)(future2->ressz));

    //sleep(3);
    thread_pool_destroy(pool);

    free(pool);
    free(future->res);
    free(future);
    free(future1->res);
    free(future1);
    //free(a);



    return 0;
}