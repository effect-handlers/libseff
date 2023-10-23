#include "seff.h"

#include <stdio.h>

DEFINE_EFFECT(put, 0, void, { int64_t value; });
DEFINE_EFFECT(get, 1, int64_t, {});

void put(seff_coroutine_t *handler, int64_t number) { PERFORM_DIRECT(handler, put, number); }
int64_t get(seff_coroutine_t *handler) { return PERFORM_DIRECT(handler, get); }

void *stateful(seff_coroutine_t *self, void *_arg) {
    seff_coroutine_t *put_handler = seff_locate_handler(EFF_ID(put));
    seff_coroutine_t *get_handler = seff_locate_handler(EFF_ID(get));
    for (int i = 0; i < 10000000; i++) {
        put(put_handler, get(get_handler) + 1);
    }
    return NULL;
}

int main(int argc, char **argv) {
    seff_coroutine_t *k = seff_coroutine_new(stateful, NULL);

    int64_t value = 0;

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

    printf("Final value is %ld\n", value);
}
