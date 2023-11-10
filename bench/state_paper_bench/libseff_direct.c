#include "seff.h"
#include "seff_types.h"

#include <stdio.h>

DEFINE_EFFECT(put, 0, void, { int64_t value; });
DEFINE_EFFECT(get, 1, int64_t, {});
DEFINE_EFFECT(error, 2, void, {});

void put(seff_coroutine_t *handler, int64_t number) { YIELD(handler, put, number); }
int64_t get(seff_coroutine_t *handler) { return YIELD(handler, get); }

void *stateful(void *_arg) {
    int depth = (uintptr_t)_arg;
    if (depth == 0) {
        seff_coroutine_t *put_handler = seff_locate_handler(EFF_ID(put));
        seff_coroutine_t *get_handler = seff_locate_handler(EFF_ID(get));

        for (int i = get(get_handler); i > 0; i = get(get_handler)) {
            put(put_handler, i - 1);
        }
    } else {
        seff_coroutine_t *k = seff_coroutine_new(stateful, (void *)(uintptr_t)(depth - 1));
        seff_handle(k, NULL, HANDLES(error));
        seff_coroutine_delete(k);
    }
    return NULL;
}

int64_t run_benchmark(int iterations, int depth) {
    seff_coroutine_t *k = seff_coroutine_new(stateful, (void *)(uintptr_t)depth);

    int64_t value = iterations;

    seff_request_t request = seff_handle(k, NULL, HANDLES(put) | HANDLES(get));
    while (!seff_finished(request)) {
        switch (request.effect) {
            CASE_EFFECT(request, put, {
                value = payload.value;
                request = seff_handle(k, NULL, HANDLES(put) | HANDLES(get));
                break;
            });
            CASE_EFFECT(request, get, {
                request = seff_handle(k, (void *)value, HANDLES(put) | HANDLES(get));
                break;
            });
        }
    }
    return value;
}

#include "common_main.inc"