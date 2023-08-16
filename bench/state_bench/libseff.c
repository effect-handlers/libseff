#include "seff.h"
#include "seff_types.h"

#include <stdio.h>

DEFINE_EFFECT(put, 0, void, { int64_t value; });
DEFINE_EFFECT(get, 1, int64_t, {});

void put(seff_coroutine_t *self, int64_t number) { PERFORM(put, number); }
int64_t get(seff_coroutine_t *self) { return PERFORM(get); }

void *stateful(seff_coroutine_t *self, void *_arg) {
    for (int i = 0; i < 10000000; i++) {
        put(self, get(self) + 1);
    }
    return NULL;
}

int main(int argc, char **argv) {
    seff_coroutine_t *k = seff_coroutine_new(stateful, NULL);

    int64_t value = 0;

    seff_eff_t *request = seff_handle(k, NULL, HANDLES(put) | HANDLES(get));
    while (k->state != FINISHED) {
        switch (request->id) {
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
