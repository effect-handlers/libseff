#include "seff.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

DEFINE_EFFECT(runtime_error, 10, void, { char *msg; });

#define MAX_DEPTH 100

#define KB (1024)

void *computation(void *_arg) {
    int64_t depth = (int64_t)_arg;
    if (depth == 0) {
        THROW(runtime_error, "error");
    } else {
        seff_coroutine_t *k = seff_coroutine_new(computation, (void *)(depth - 1));
        seff_resume(k, NULL, 0);
        seff_coroutine_delete(k);
    }
    return NULL;
}

int main(void) {
    size_t caught = 0;

    for (size_t i = 0; i < 100000; i++) {
        seff_coroutine_t *k = seff_coroutine_new(computation, (void *)(MAX_DEPTH - 1));
        seff_request_t exn = seff_resume(k, NULL, HANDLES(runtime_error));
        switch (exn.effect) {
            CASE_EFFECT(exn, runtime_error, { caught++; });
            break;
        default:
            assert(false);
        }
        seff_coroutine_delete(k);
    }
    printf("Caught %lu exceptions\n", caught);
    return 0;
}
