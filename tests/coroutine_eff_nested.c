#include "seff.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Effects: print, read
 */

DEFINE_EFFECT(print, 0, void, { char *string; });
DEFINE_EFFECT(read, 1, char *, {});

void bounce(const char *name) {
    const char *received_msg = PERFORM(read);
    char response[100];
    size_t i = 0;
    size_t j = 0;
    while (name[j] != 0) {
        response[i] = name[j];
        i++;
        j++;
    }
    j = 0;
    while (" received "[j] != 0) {
        response[i] = " received "[j];
        i++;
        j++;
    }
    j = 0;
    while (received_msg[j] != 0) {
        response[i] = received_msg[j];
        i++;
        j++;
    }
    response[i] = 0;
    PERFORM(print, response);
}

void *nested(void *args) {
    while (1) {
        bounce("nested");
    }

    return NULL;
}

void *parent(void *args) {
    seff_coroutine_t *toplevel = seff_current_coroutine();
    seff_coroutine_t *child = seff_coroutine_new(nested, toplevel);
    seff_handle(child, NULL, HANDLES(read));

    for (size_t i = 0; i < 3; i++) {
        bounce("parent");
        char i_str[2];
        i_str[0] = '0' + i;
        i_str[1] = 0;
        seff_handle(child, i_str, HANDLES(read));
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(parent, NULL);

    seff_request_t request = seff_handle(k, NULL, HANDLES(print) | HANDLES(read));
    while (!seff_finished(request)) {
        void *response;
        switch (request.effect) {
            CASE_EFFECT(request, print, {
                puts(payload.string);
                response = NULL;
                break;
            });
            CASE_EFFECT(request, read, {
                response = "TOPLEVEL MESSAGE";
                break;
            });
        default:
            assert(false);
            exit(-1);
        }
        request = seff_handle(k, response, HANDLES(print) | HANDLES(read));
    }
    seff_coroutine_delete(k);
}
