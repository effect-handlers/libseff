#include "seff.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

DEFINE_EFFECT(runtime_error, 10, void, { char *msg; });

void *computation(seff_coroutine_t *self, void *_arg) { THROW(runtime_error, "error"); }

int main(void) {
    size_t caught = 0;
    for (size_t i = 0; i < 1000000; i++) {
        seff_coroutine_t *k = seff_coroutine_new(computation, NULL);
        seff_eff_t *exn = seff_handle(k, NULL, HANDLES(runtime_error));
        switch (exn->id) {
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
