#include "seff.h"
#include "seff_types.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

DEFINE_EFFECT(print, 0, void, { const char *str; });
void print(const char *str) { PERFORM(print, str); }

void *default_print(void *_arg) {
    EFF_PAYLOAD_T(print) payload = *(EFF_PAYLOAD_T(print) *)_arg;
    fputs(payload.str, stdout);
    return NULL;
}

extern size_t default_frame_size;
void *with_output_to_buffer(seff_start_fun_t *fn, void *arg, char *buffer) {
    seff_coroutine_t k;
    seff_coroutine_init(&k, fn, arg);
    seff_resumption_t res = seff_coroutine_start(&k);

    char *buffptr = buffer;
    while (true) {
        seff_eff_t *req = seff_handle(res, NULL, HANDLES(print));
        if (k.state == FINISHED) {
            return (void *)req;
        }
        switch (req->id) {
            CASE_EFFECT(req, print, {
                // Note we are NOT using strcpy!
                buffptr = stpcpy(buffptr, payload.str);
                res = req->resumption;
                break;
            });
        default:
            assert(false);
        }
    }

    return NULL;
}

void *with_output_to_default(seff_start_fun_t *fn, void *arg) {
    seff_coroutine_t k;
    seff_coroutine_init(&k, fn, arg);

    seff_eff_t *req = seff_handle(seff_coroutine_start(&k), NULL, 0);
    assert(k.state == FINISHED);
    return (void *)req;
}

void *printer(seff_coroutine_t *self, void *arg) {
    print("Hello, ");
    print("World!\n");
    return NULL;
}

int main(void) {
    seff_set_default_handler(EFF_ID(print), default_print);
    with_output_to_default(printer, NULL);
    char buffer[512] = {0};
    with_output_to_buffer(printer, NULL, buffer);
    printf("Coroutine said %s", buffer);
}
