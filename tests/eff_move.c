#include "seff.h"
#include "seff_types.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_EFFECT(get_name, 0, char *, {});
DEFINE_EFFECT(print, 1, void, { char *str; });

void *foo(seff_coroutine_t *self, void *arg) {
    char str[128] = "Hello from ";

    while (1) {
        char *back = str + sizeof("Hello from ") - 1;
        char *name = PERFORM(get_name);
        strcpy(back, name);
        PERFORM(print, str);
    }
}

void *bar(seff_coroutine_t *self, void *_child) {
    seff_coroutine_t *child = (seff_coroutine_t *)_child;

    char *response = NULL;
    seff_request_t request = seff_handle(child, response, HANDLES(get_name));
    switch (request.effect) {
        CASE_EFFECT(request, get_name, {
            response = "bar1";
            break;
        });
    default:
        assert(false);
    }

    request = seff_handle(child, response, HANDLES(get_name));
    switch (request.effect) {
        CASE_EFFECT(request, get_name, {
            response = "bar2";
            break;
        });
    default:
        assert(false);
    }

    request = seff_handle(child, response, HANDLES(get_name));
    switch (request.effect) {
        CASE_EFFECT(request, get_name, {
            response = "bar3";
            break;
        });
    default:
        assert(false);
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(foo, NULL);

    seff_request_t request = seff_handle(k, NULL, HANDLES(get_name) | HANDLES(print));
    assert(request.effect == EFF_ID(get_name));

    char *main_name = "main";
    request = seff_handle(k, main_name, HANDLES(get_name) | HANDLES(print));
    assert(request.effect == EFF_ID(print));
    puts(((EFF_PAYLOAD_T(print) *)request.payload)->str);

    seff_coroutine_t *j = seff_coroutine_new(bar, k);

    while (1) {
        request = seff_handle(j, NULL, HANDLES(print));
        if (j->state == FINISHED)
            break;
        switch (request.effect) {
            CASE_EFFECT(request, print, {
                puts(payload.str);
                break;
            });
        default:
            assert(false);
        }
    }
}
