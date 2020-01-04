#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"
#include "future.h"

#define THREADS 4



void* calc_value(void* arg, size_t argsz, size_t* size) {
    int* args = arg;
    usleep((unsigned)args[1]);
    int* res = malloc(sizeof(int));
    (*res) = args[0];
    *size = 1;
    return res;
}

void* calc_row(void* arg, size_t argsz, size_t* size) {
    int* sum = malloc(sizeof(int));
    *sum = 0;
    future_t** args = arg;
    for (int i = 0; i < argsz; i++) {
        int* res = await(args[i]);
        (*sum) += (*res);
        free(res);
    }
    (*size) = 1;
    return sum;
}


int main() {
    unsigned n, k;
    scanf("%d", &n);
    scanf("%d", &k);
    future_t* tab[n][k];
    callable_t call_tab[n][k];
    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, THREADS);

    //reading with lag
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k; j++) {
            tab[i][j] = malloc(sizeof(future_t));
            callable_t call;
            int* arg = malloc(sizeof(int)*2);
            call.argsz = 2;
            call.function = &calc_value;
            scanf("%d", arg);
            scanf("%d", arg + 1);
            call.arg = arg;
            call_tab[i][j] = call;
            async(pool, tab[i][j], call_tab[i][j]);
        }
    }

    //multi-thread row sums counting
    future_t** row_sums = malloc(sizeof(future_t*)*n);
    for (int i = 0; i < n; i++) {
        row_sums[i] = malloc(sizeof(future_t));
        callable_t call;
        call.arg = tab[i];
        call.argsz = k;
        call.function = calc_row;
        async(pool, row_sums[i], call);
    }

    //printing row sums in correct order
    for (int i = 0; i < n; i++) {
        int * a = await(row_sums[i]);
        printf("%d\n", *a);
        free(a);
    }

    //memory freeing
    for (int i = 0; i < n; i++) {
        free(row_sums[i]);
        for (int j = 0; j < k; j++) {
            free(tab[i][j]);
            free(call_tab[i][j].arg);
        }
    }
    free(row_sums);
    thread_pool_destroy(pool);
    free(pool);

}