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
    seff_resumption_t res = *(seff_resumption_t *)_child;

    char *response = NULL;
    seff_eff_t *request = seff_handle(res, response, HANDLES(get_name));
    switch (request->id) {
        CASE_EFFECT(request, get_name, {
            response = "bar1";
            res = request->resumption;
            break;
        });
    default:
        assert(0);
    }

    request = seff_handle(res, response, HANDLES(get_name));
    switch (request->id) {
        CASE_EFFECT(request, get_name, {
            response = "bar2";
            res = request->resumption;
            break;
        });
    default:
        assert(0);
    }

    request = seff_handle(res, response, HANDLES(get_name));
    switch (request->id) {
        CASE_EFFECT(request, get_name, {
            response = "bar3";
            res = request->resumption;
            break;
        });
    default:
        assert(0);
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(foo, NULL);
    seff_resumption_t res = seff_coroutine_start(k);

    seff_eff_t *request = seff_resume(res, NULL);
    assert(request->id == EFF_ID(get_name));
    res = request->resumption;

    char *main_name = "main";
    request = seff_resume(res, main_name);
    assert(request->id == EFF_ID(print));
    puts(((EFF_PAYLOAD_T(print) *)request->payload)->str);
    res = request->resumption;

    seff_coroutine_t *j = seff_coroutine_new(bar, &res);
    seff_resumption_t res_j = seff_coroutine_start(j);

    while (1) {
        request = seff_handle(res_j, NULL, HANDLES(print));
        if (j->state == FINISHED)
            break;
        switch (request->id) {
            CASE_EFFECT(request, print, {
                puts(payload.str);
                res_j = request->resumption;
                break;
            });
        default:
            assert(0);
        }
    }
}
