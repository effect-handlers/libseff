#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int64_t stacky(seff_coroutine_t *k, int64_t a) {
    if (a) {
        return stacky(k, a - 1);
    }
    seff_return(k, (void *)54321);
    return 1;
}

void *fn(seff_coroutine_t *self, void *_arg) {
    int64_t arg = (int64_t)_arg;

    return (void *)stacky(self, arg);
}

#define REPS 1
int main(void) {
    for (int i = 0; i < REPS; i++) {
        seff_coroutine_t *k = seff_coroutine_new(fn, (void *)(10 * 1000));
        int64_t res = (int64_t)seff_resume(k, NULL);

        printf("RES: %ld\n", res);
        seff_coroutine_delete(k);
    }
}
