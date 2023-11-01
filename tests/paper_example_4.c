#include "seff.h"
#include "seff_types.h"

#include <stdio.h>

typedef enum { add_op, sub_op, mul_op, div_op } op2_t;

typedef struct {
    double v;
    double *dv;
} backprop_t;

DEFINE_EFFECT(r_ap0, 3, backprop_t *, { double value; });
DEFINE_EFFECT(r_ap2, 5, backprop_t *, {
    op2_t op;
    backprop_t arg1;
    backprop_t arg2;
});

effect_set r_smooth = HANDLES(r_ap0) | HANDLES(r_ap2);

backprop_t constant(double x) { return *PERFORM(r_ap0, x); }
backprop_t add(backprop_t x, backprop_t y) { return *PERFORM(r_ap2, add_op, x, y); }
backprop_t sub(backprop_t x, backprop_t y) { return *PERFORM(r_ap2, sub_op, x, y); }
backprop_t mul(backprop_t x, backprop_t y) { return *PERFORM(r_ap2, mul_op, x, y); }
backprop_t divide(backprop_t x, backprop_t y) { return *PERFORM(r_ap2, div_op, x, y); }

backprop_t result;
typedef struct {
    backprop_t x;
    size_t iters;
} newton_sqrt_args_t;
void *newton_sqrt(void *args) {
    backprop_t x = ((newton_sqrt_args_t *)args)->x;
    size_t iters = ((newton_sqrt_args_t *)args)->iters;

    backprop_t two = constant(2.0);
    backprop_t acc = constant(1.0);
    for (size_t i = 0; i < iters; i++) {
        backprop_t numerator = sub(mul(acc, acc), x);
        backprop_t denominator = mul(two, acc);
        acc = sub(acc, divide(numerator, denominator));
    }

    result = acc;
    return &result;
}

void handle(seff_coroutine_t *k, backprop_t *response) {
    seff_request_t request = seff_handle(k, response, HANDLES(r_ap0) | HANDLES(r_ap2));
    switch (request.effect) {
        CASE_RETURN(request, {
            *((backprop_t*)payload.result)->dv = 1.0;
            return;
        })
        CASE_EFFECT(request, r_ap0, {
            double v = payload.value;
            double dv = 0.0;
            backprop_t r = ((backprop_t){v, &dv});
            handle(k, &r);

            break;
        })
        CASE_EFFECT(request, r_ap2, {
            double v;
            switch (payload.op) {
            case add_op:
                v = payload.arg1.v + payload.arg2.v;
                break;
            case sub_op:
                v = payload.arg1.v - payload.arg2.v;
                break;
            case mul_op:
                v = payload.arg1.v * payload.arg2.v;
                break;
            case div_op:
                v = payload.arg1.v / payload.arg2.v;
                break;
            }
            double dv = 0.0;
            backprop_t r = ((backprop_t){v, &dv});
            handle(k, &r);

            double x = payload.arg1.v;
            double y = payload.arg2.v;
            double *dx = payload.arg1.dv;
            double *dy = payload.arg2.dv;
            switch (payload.op) {
            case add_op:
                *dx = *dx + dv;
                *dy = *dy + dv;
                break;
            case sub_op:
                *dx = *dx + dv;
                *dy = *dy - dv;
                break;
            case mul_op:
                *dx = *dx + (y * dv);
                *dy = *dy + (x * dv);
                break;
            case div_op:
                *dx = *dx + (dv / y);
                *dy = *dy - ((x * dv) / (y * y));
                break;
            }
            break;
        })
    }
}

int main(int argc, char **argv) {
    double dx = 0.0;
    backprop_t x = (backprop_t){.v = 4.0, .dv = &dx};
    newton_sqrt_args_t args = (newton_sqrt_args_t){.x = x, .iters = 160};
    seff_coroutine_t *r = seff_coroutine_new(newton_sqrt, &args);
    handle(r, NULL);
    printf("sqrt(4) = %lf\n", result.v);
    printf("sqrt'(4) = %lf\n", *x.dv);
    seff_coroutine_delete(r);
    return 0;
}
