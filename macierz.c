#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"
#include "future.h"

#define THREADS 4

typedef struct pair {
    int first;
    int second;
} pair_t;

void* calc_row(void* arg, size_t argsz, size_t* size) {
    int* sum = malloc(sizeof(int));
    *sum = 0;
    pair_t* args = arg;
    for (unsigned i = 0; i < argsz; i++) {
        usleep((unsigned)args[i].second*1000);
        (*sum) += args[i].first;
    }
    (*size) = 1;
    return sum;
}

int main() {
    unsigned n, k;
    scanf("%d", &n);
    scanf("%d", &k);
    pair_t tab[n][k];
    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, THREADS);

    //reading input
    for (unsigned i = 0; i < n; i++) {
        for (unsigned j = 0; j < k; j++) {
            int a, b;
            scanf("%d", &a);
            scanf("%d", &b);
            pair_t in;
            in.first = a;
            in.second = b;
            tab[i][j] = in;
        }
    }

    //multi-thread row sums counting
    future_t** row_sums = malloc(sizeof(future_t*)*n);
    for (unsigned i = 0; i < n; i++) {
        row_sums[i] = malloc(sizeof(future_t));
        callable_t call;
        call.arg = tab[i];
        call.argsz = k;
        call.function = calc_row;
        async(pool, row_sums[i], call);
    }

    //printing row sums in correct order
    for (unsigned i = 0; i < n; i++) {
        int * a = await(row_sums[i]);
        printf("%d\n", *a);
        free(a);
    }

    //memory freeing
    for (unsigned i = 0; i < n; i++) {
        free(row_sums[i]);
    }
    free(row_sums);
    thread_pool_destroy(pool);
    free(pool);

}