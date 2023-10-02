#include "seff.h"
#include <alloca.h>
#include <stdio.h>

void heavy_hello(void) {
    char a[8000];
    a[7998] = 'a';
    puts("Hello world! I am a coroutine!");
}

void *deep_hello(seff_coroutine_t *k, void *_arg) {
    int64_t arg = (int64_t)_arg;
    if (arg--) {
        seff_coroutine_t *q = seff_coroutine_new(deep_hello, (void *)arg);
        seff_resumption_t res = seff_coroutine_start(q);
        seff_resume(res, (void *)arg);
        seff_coroutine_delete(q);
    } else {
        heavy_hello();
    }
    return NULL;
}

int main(void) {
    int reps = 100;
    printf("Creating %d coroutines", reps);
    while (reps--) {
        seff_coroutine_t *k = seff_coroutine_new(deep_hello, (void *)1000);
        seff_resumption_t res = seff_coroutine_start(k);
        seff_resume(res, NULL);
        seff_coroutine_delete(k);

        // This moves the stack a bit between repetitions
        alloca(400);
    }
}
