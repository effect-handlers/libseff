#include "seff.h"
#include <stdio.h>

DEFINE_EFFECT(defer, 0, void, {
    void (*defer_fn)(void *);
    void *defer_arg;
});

DEFINE_EFFECT(yield_str, 1, void, { char *elt; });

void *allocate(size_t size) {
    void *ptr = malloc(size);
    PERFORM(defer, free, ptr);
    return ptr;
}

void *handle_defer(seff_coroutine_t *k) {
    seff_request_t req = seff_handle(k, NULL, HANDLES(defer));
    switch (req.effect) {
        CASE_EFFECT(req, defer, {
            void *result = handle_defer(k);
            payload.defer_fn(payload.defer_arg);
            return result;
        })
        CASE_RETURN(req, { return payload.result; });
    default:
        exit(-1);
    }
}

void *squares(void *arg) {
    int64_t limit = (size_t)arg;
    for (int64_t i = 0; i < limit; i++) {
        char *str = allocate(32);
        sprintf(str, "%5lu", i * i);
        PERFORM(yield_str, str);
    }
    return NULL;
}

void *print_all(void *_arg) {
    seff_coroutine_t *k = seff_coroutine_new(squares, (void *)50);
    while (true) {
        seff_request_t req = seff_handle(k, NULL, HANDLES(yield_str));
        switch (req.effect) {
            CASE_EFFECT(req, yield_str, {
                puts(payload.elt);
                break;
            })
            CASE_RETURN(req, {
                seff_coroutine_delete(k);
                return NULL;
            })
        }
    }
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(print_all, NULL);
    handle_defer(k);
    seff_coroutine_delete(k);
}
