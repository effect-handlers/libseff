#include "seff.h"
#include <assert.h>
#include <stdio.h>

/*
 * Effects: print, read
 */

#define PRINT_CODE 0
#define READ_CODE 1

void *effectful_body(void *args) {
    // perform(read)
    seff_coroutine_t *self = seff_current_coroutine();

    while (1) {
        seff_yield(self, READ_CODE, NULL);
        // printf("[COROUTINE]: Received %s\n", result);

        char *msg = "Thanks for your message!";
        seff_yield(self, PRINT_CODE, msg);
    }

    return NULL;
}

int main(void) {
    seff_coroutine_t *k = seff_coroutine_new(effectful_body, NULL);

    seff_request_t request = seff_resume(k, NULL);

    for (size_t i = 0; i < 5; i++) {
        switch (request.effect) {
        case PRINT_CODE:
            puts((char*)request.payload);
            request = seff_resume(k, NULL);
            break;
        case READ_CODE:
            request = seff_resume(k, "MSG");
            break;
        default:
            assert(false);
        }
    }
}
