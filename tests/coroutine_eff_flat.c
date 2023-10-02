#include "seff.h"
#include <assert.h>
#include <stdio.h>

/*
 * Effects: print, read
 */

#define PRINT_CODE 0
#define READ_CODE 1
typedef struct {
    int64_t code;
    void *payload;
} eff;

void *effectful_body(seff_coroutine_t *k, void *args) {
    // perform(read)
    eff read = {.code = READ_CODE};

    while (1) {
        seff_yield(k, &read);
        // printf("[COROUTINE]: Received %s\n", result);

        char *msg = "Thanks for your message!";
        eff write = {.code = PRINT_CODE, .payload = msg};
        seff_yield(k, &write);
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(effectful_body, NULL);
    seff_resumption_t res = seff_coroutine_start(k);

    eff *request = seff_resume(res, NULL);

    for (size_t i = 0; i < 5; i++) {
        res.sequence += 1;
        switch (request->code) {
        case PRINT_CODE:
            puts(request->payload);
            request = seff_resume(res, NULL);
            break;
        case READ_CODE:
            request = seff_resume(res, "MSG");
            break;
        default:
            assert(0);
        }
    }
}
