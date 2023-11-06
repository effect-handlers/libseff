#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int64_t stacky(int64_t a) {
    if (a) {
        return stacky(a - 1);
    }
    //PERFORM_DIRECT(k, return, (void *)54321);
    seff_exit(seff_current_coroutine(), EFF_ID(return), (void*)54321);
    return 1;
}

void *fn(void *_arg) {
    int64_t arg = (int64_t)_arg;

    return (void *)stacky(arg);
}

#define REPS 1
int main(void) {
    for (int i = 0; i < REPS; i++) {
        seff_coroutine_t *k = seff_coroutine_new(fn, (void *)(10 * 1000));

        seff_request_t req = seff_resume(k, NULL);
        assert(req.effect == EFF_ID(return));
        int64_t ret = (int64_t)req.payload;

        printf("RES: %ld\n", ret);
        assert(ret == 54321);
        if (ret != 54321) {
            exit(-1);
        }
        seff_coroutine_delete(k);
    }
}
