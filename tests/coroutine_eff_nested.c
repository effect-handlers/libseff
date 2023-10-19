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

void *nested(seff_coroutine_t *read_handler, void *args) {
    while (1) {
        bounce("nested");
    }

    return NULL;
}

void *parent(seff_coroutine_t *toplevel_handler, void *args) {
    seff_coroutine_t *child = seff_coroutine_new(nested, toplevel_handler);
    seff_resumption_t res = seff_coroutine_start(child);

    seff_eff_t *request = seff_handle(res, NULL, HANDLES(read));
    res = request->resumption;

    for (size_t i = 0; i < 3; i++) {
        bounce("parent");
        char i_str[2];
        i_str[0] = '0' + i;
        i_str[1] = 0;
        request = seff_handle(res, i_str, HANDLES(read));
        res = request->resumption;
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(parent, NULL);

    seff_eff_t *request = seff_resume(seff_coroutine_start(k), NULL);
    while (k->state != FINISHED) {
        void *response;
        switch (request->id) {
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
            assert(0);
            return -1;
        }
        request = seff_resume(request->resumption, response);
    }
}
