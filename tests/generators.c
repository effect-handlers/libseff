#include "seff.h"
#include <stdio.h>

DEFINE_EFFECT(yield_str, 1, void, { char *elt; });

void *squares(void *arg) {
    int64_t limit = (size_t)arg;
    for (int64_t i = 0; i < limit; i++) {
        char *str = malloc(32);
        sprintf(str, "%5lu", i * i);
        PERFORM(yield_str, str);
    }
    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(squares, (void *)50);
    while (true) {
        seff_request_t req = seff_handle(k, NULL, HANDLES(yield_str));
        switch (req.effect) {
            CASE_EFFECT(req, yield_str, {
                puts(payload.elt);
                free(payload.elt);
                break;
            })
            CASE_RETURN(req, {
                seff_coroutine_delete(k);
                return 0;
            })
        }
    }
}
