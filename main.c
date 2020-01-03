#include <stdio.h>
#include <stdlib.h>
#include "threadpool.h"

int main() {

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 5);
    thread_pool_destroy(pool);

    return 0;
}