#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int64_t stacky(int a) {
    if (a) {
        return stacky(a - 1);
    }
    return 54321;
}

void *fn(seff_coroutine_t *self, void *_arg) {
    int64_t arg = (int64_t)_arg;

    return (void *)stacky(arg);
}

int main(void) {
    int reps = 3000;
    while (reps--) {
        seff_coroutine_t *k = seff_coroutine_new(fn, (void *)129); //(void *)(10 * 1000));
        int64_t res = (int64_t)seff_resume(seff_coroutine_start(k), NULL);

        printf("RES: %ld\n", res);
        seff_coroutine_delete(k);
    }
}
