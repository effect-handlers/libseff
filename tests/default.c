#include "seff.h"
#include "seff_types.h"

#include <assert.h>
#include <stdio.h>

DEFINE_EFFECT(division_by_zero, 10, void, { int dividend; });

int safe_division(int dividend, int divisor) {
    if (divisor == 0) {
        THROW(division_by_zero, dividend);
    }
    return dividend / divisor;
}

void *safe_computation(void *_arg) {
    for (int i = -5; i < 5; i++) {
        safe_division(100, i);
    }
    return NULL;
}

void *div_by_zero_crash(void *arg) {
    EFF_PAYLOAD_T(division_by_zero) payload = *(EFF_PAYLOAD_T(division_by_zero) *)arg;
    fprintf(stderr, "ERROR: dividing %d by zero\n", payload.dividend);
    exit(0);
}

extern size_t default_frame_size;
int main(void) {
    seff_set_default_handler(EFF_ID(division_by_zero), div_by_zero_crash);
    seff_coroutine_t k;
    seff_coroutine_init(&k, safe_computation, NULL);

    seff_request_t exn = seff_handle(&k, NULL, HANDLES(division_by_zero));
    switch (exn.effect) {
        CASE_EFFECT(exn, division_by_zero, {
            printf("Caught division (%d / 0) in coroutine, continuing main\n", payload.dividend);
            break;
        });
    default:
        assert(false);
    }
    seff_coroutine_release(&k);
    safe_division(100, 0);
    return 0;
}
