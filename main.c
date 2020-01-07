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

void* ff1(void* arg, size_t argsz, size_t* ss) {
    //sleep(1);
    printf("testy1\n");
    int* a = malloc(160);
    a[0] = 1; a[1] = 7; a[2] = 0; a[3] = 7;
    return a;
}

void* ff_1(void* arg, size_t argsz, size_t* ss) {
    //sleep(1);
    printf("testy1\n");
    int*a = arg;
    a[0] = 1; a[1] = 7; a[2] = 1; a[3] = 0;
    return a;
}



void* ff2(void* arg, size_t argsz, size_t* ss) {
    int* args = arg;
    printf("%d%d.%d%d\n", args[0], args[1], args[2], args[3]);
    free(arg);
    return NULL;
}


void ff3(void* arg, size_t argsz) {
    //sleep(4);
    //printf("testy3\n");
}

void* double_(void* arg, size_t argsz, size_t* ressz) {
    //sleep(2);
    *ressz = 18;
    int* a=  malloc(sizeof(int));
    *a = (*(int*)(arg))*2;
    return a;
}

int main() {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 2);
    thread_pool_t* pool2 = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool2, 2);
    thread_pool_t* pool3 = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool3, 2);


    future_t* f1 = malloc(sizeof(future_t));
    future_t* f2 = malloc(sizeof(future_t));
    future_t* f3 = malloc(sizeof(future_t));
    future_t* f4 = malloc(sizeof(future_t));
    future_t* f5 = malloc(sizeof(future_t));
    future_t* f6 = malloc(sizeof(future_t));



    callable_t c1;
    c1.function = &ff1;
    c1.arg = NULL;
    c1.argsz = 0;
    callable_t c2;
    c2.function = &ff1;
    c2.arg = NULL;
    c2.argsz = 0;

    async(pool, f1, c1);
    map(pool2, f2, f1, &ff_1);
    map(pool3, f3, f2, &ff2);




/*
    runnable_t r3;
    r3.function = &ff3;
    r3.argsz = 0;
    r3.arg = NULL;

    for (int i = 0; i < 10; i++) {
        defer(pool, r3);
    }

*/

    thread_pool_destroy(pool);
    thread_pool_destroy(pool2);
    thread_pool_destroy(pool3);

    free(pool);
    free(pool2);
    free(pool3);
    free(f1);
    free(f2);
    free(f3);
    free(f4);
    free(f5);
    free(f6);

    return 0;
}