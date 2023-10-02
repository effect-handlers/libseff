#include "seff.h"
#include "seff_types.h"
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
    int metaReps = 60000;
    void *res;
    while (metaReps--) {

        int reps = 1;
        while (reps < 100) {
            res = stacky((void *)&reps);
            reps++;
        }
    }

    printf("%ld", (int64_t)res);

    return NULL;
}

int main(void) {
    int reps = 1;
    while (reps--) {
        seff_coroutine_t *a = seff_coroutine_new(coroutine_a, NULL);
        seff_resumption_t res = seff_coroutine_start(a);
        seff_resume(res, NULL);
        seff_coroutine_delete(a);
    }
}
