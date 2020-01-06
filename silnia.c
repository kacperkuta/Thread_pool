#include "future.h"

void* count(void* arg, size_t argsz, size_t* size) {
    unsigned long long* res = malloc(2*sizeof(unsigned long long));
    unsigned long long* args = arg;
    if (args[1] != 1) {
        res[0] = args[0] * args[1];
    } else {
        res[0] = 1;
    }
    res[1] = args[1] + 1;
    free(arg);
    return res;
}

int main() {

    int n;
    scanf("%d", &n);
    future_t* future_tab[n];

    thread_pool_t* pool = malloc(sizeof(thread_pool_t));
    thread_pool_init(pool, 3);

    for (int i = 0; i < n; i++) {
        future_tab[i] = malloc(sizeof(future_t));

        if (i == 0) {
            unsigned long long *args = malloc(2 * sizeof(unsigned long long));
            args[0] = 0;
            args[1] = 1;
            callable_t call;
            call.arg = args;
            call.argsz = 2;
            call.function = count;
            async(pool, future_tab[0], call);
        } else {
            map(pool, future_tab[i], future_tab[i - 1], count);
        }
    }

    unsigned long long* res = await(future_tab[n-1]);
    printf("%llu\n", res[0]);
    free(res);

    for (int i = 0; i < n; i++) {
        free(future_tab[i]);
    }
    thread_pool_destroy(pool);
    free(pool);

}