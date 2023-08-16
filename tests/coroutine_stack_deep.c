#include "seff.h"
#include <stdio.h>

void *stacky(void *a) {
    int m = *(int *)a;
    m--;
    if (m) {
        void *res = stacky((void *)&m);

        return (void *)((int64_t)res + 1);
    }
    return (void *)73330;
}

void *coroutine_a(seff_coroutine_t *self, void *_nil) {
    void *res;
    int reps = 10000000; // 1000;

    res = stacky((void *)&reps);

    printf("%ld", (int64_t)res);

    return NULL;
}

int main(void) {
    int reps = 1;
    while (reps--) {
        seff_coroutine_t *a = seff_coroutine_new(coroutine_a, NULL);
        seff_resume(a, NULL);
        seff_coroutine_delete(a);
    }
}
