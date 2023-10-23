#include "seff.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

DEFINE_EFFECT(runtime_error, 10, void, { char *msg; });

#define MAX_DEPTH 100

#define KB (1024)

seff_coroutine_t children[MAX_DEPTH];
size_t caught = 0;

void *computation(seff_coroutine_t *self, void *_arg) {
    int64_t depth = (int64_t)_arg;
    if (depth == 0) {
        THROW(runtime_error, "error");
    } else {
        seff_coroutine_init(&children[depth], computation, (void *)(depth - 1));
        seff_request_t exn = seff_handle(&children[depth], NULL, HANDLES(runtime_error));
        switch (exn.effect) {
            CASE_EFFECT(exn, runtime_error, {
                caught++;
                THROW(runtime_error, payload.msg);
                break;
            });
        default:
            assert(false);
        }
    }
    return NULL;
}

int main(void) {
    seff_coroutine_t k;
    seff_coroutine_init_sized(&k, computation, (void *)(MAX_DEPTH - 1), 16 * KB);
    for (size_t depth = 0; depth < MAX_DEPTH; depth++) {
        seff_coroutine_init_sized(&children[depth], computation, NULL, 16 * KB);
    }

    for (size_t i = 0; i < 100000; i++) {
        seff_request_t exn = seff_handle(&k, NULL, HANDLES(runtime_error));
        switch (exn.effect) {
            CASE_EFFECT(exn, runtime_error, {
                caught++;
                break;
            });
            break;
        default:
            assert(false);
        }
        seff_coroutine_init_sized(&k, computation, (void *)(MAX_DEPTH - 1), 16 * KB);
    }
    seff_coroutine_release(&k);
    for (size_t depth = 0; depth < MAX_DEPTH; depth++) {
        seff_coroutine_release(&children[depth]);
    }
    printf("Caught %lu exceptions\n", caught);
    return 0;
}
