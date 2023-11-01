#include "seff.h"
#include <stdio.h>

DEFINE_EFFECT(get, 0, int64_t, {});
DEFINE_EFFECT(put, 1, void, { int64_t new_value; });

void *counter(void *parameter) {
    int64_t counter;
    do {
        counter = PERFORM(get);
        printf("Counter is %ld\n", counter);
        PERFORM(put, counter - 1);
    } while (counter > 0);

    return NULL;
}

int main(void) {
    effect_set handles_state = HANDLES(get) | HANDLES(put);

    seff_coroutine_t *k = seff_coroutine_new(counter, NULL);
    seff_request_t req = seff_handle(k, NULL, handles_state);
    int64_t state = 100;
    bool done = false;
    while (!done) {
        switch (req.effect) {
            CASE_EFFECT(req, get, {
                req = seff_handle(k, (void *)state, handles_state);
                break;
            });
            CASE_EFFECT(req, put, {
                state = payload.new_value;
                req = seff_handle(k, NULL, handles_state);
                break;
            });
            CASE_RETURN(req, {
                printf("The handled code has finished executing\n");
                done = true;
                break;
            });
        }
    }

    seff_coroutine_delete(k);
    return 0;
}
