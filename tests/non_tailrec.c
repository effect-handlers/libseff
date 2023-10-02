#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_EFFECT(put, 0, void, { int x; });

void *effectful(seff_coroutine_t *self, void *arg) {
    size_t iters = (size_t)arg;
    for (size_t i = 0; i < iters; i++) {
        PERFORM(put, i);
    }
    return NULL;
}

void dummy(void) __attribute__((optnone)) { free(NULL); }

void handle(seff_resumption_t res) {
    seff_eff_t *request = seff_handle(res, NULL, HANDLES(put));
    if (res.coroutine->state == FINISHED)
        return;
    switch (request->id) {
        CASE_EFFECT(request, put, {
            dummy();
            handle(request->resumption);
            dummy();
        });
    }
}

int main(int argc, char **argv) {
    size_t depth = 100;
    if (argc == 2) {
        sscanf(argv[1], "%lu", &depth);
    }
    printf("depth is %lu\n", depth);

    seff_coroutine_t *k = seff_coroutine_new(effectful, (void *)depth);

    handle(seff_coroutine_start(k));
    return 0;
}
