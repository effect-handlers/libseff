#include "seff.h"
#include "seff_types.h"

#include <stdio.h>

typedef enum { negate_op } op1_t;
typedef enum { add_op, multiply_op } op2_t;

DEFINE_EFFECT(e_ap0, 0, double *, { double value; });
DEFINE_EFFECT(e_ap1, 1, double *, {
    op1_t op;
    double arg1;
});
DEFINE_EFFECT(e_ap2, 2, double *, {
    op2_t op;
    double arg1;
    double arg2;
});

typedef struct {
    double v;
    double *dv;
} prop_t;

effect_set e_smooth = HANDLES(e_ap0) | HANDLES(e_ap1) | HANDLES(e_ap2);

double e_c(double x) { return *PERFORM(e_ap0, x); }
double e_n(double x) { return *PERFORM(e_ap1, negate_op, x); }
double e_a(double x, double y) { return *PERFORM(e_ap2, add_op, x, y); }
double e_m(double x, double y) { return *PERFORM(e_ap2, multiply_op, x, y); }

DEFINE_EFFECT(r_ap0, 3, prop_t *, { double value; });
DEFINE_EFFECT(r_ap1, 4, prop_t *, {
    op1_t op;
    prop_t arg1;
});
DEFINE_EFFECT(r_ap2, 5, prop_t *, {
    op2_t op;
    prop_t arg1;
    prop_t arg2;
});

effect_set r_smooth = HANDLES(r_ap0) | HANDLES(r_ap1) | HANDLES(r_ap2);

prop_t r_c(double x) { return *PERFORM(r_ap0, x); }
prop_t r_n(prop_t x) { return *PERFORM(r_ap1, negate_op, x); }
prop_t r_a(prop_t x, prop_t y) { return *PERFORM(r_ap2, add_op, x, y); }
prop_t r_m(prop_t x, prop_t y) { return *PERFORM(r_ap2, multiply_op, x, y); }

prop_t result;
prop_t x;

void *example(seff_coroutine_t *self, void *args) {
    size_t iters = (size_t)args;

    printf("iters: %lu\n", iters);

    double dx = 0.0;
    x = (prop_t){0.5, &dx};

    prop_t acc = r_c(1.0);
    prop_t prev = r_c(1.0);
    for (size_t i = 0; i < iters; i++) {
        prev = r_m(prev, r_n(r_a(x, r_c(-1.0))));
        acc = r_a(acc, prev);
    }

    result = acc;

    return NULL;
}

void handle(seff_coroutine_t *k, prop_t *response) {
    seff_eff_t *request = seff_handle(k, response, r_smooth);
    if (k->state == FINISHED) {
        *result.dv = 1.0;
        return;
    }
    switch (request->id) {
        CASE_EFFECT(request, r_ap0, {
            double v = e_c(payload.value);
            double dv = 0.0;
            prop_t r = ((prop_t){v, &dv});
            handle(k, &r);

            break;
        })
        CASE_EFFECT(request, r_ap1, {
            double v;
            switch (payload.op) {
            case negate_op:
                v = e_n(payload.arg1.v);
                break;
            }
            double dv = 0.0;
            prop_t r = ((prop_t){v, &dv});
            handle(k, &r);

            double *dx = payload.arg1.dv;
            switch (payload.op) {
            case negate_op:
                *dx = e_a(*dx, e_n(dv));
                break;
            }
            break;
        })
        CASE_EFFECT(request, r_ap2, {
            double v;
            switch (payload.op) {
            case add_op:
                v = e_a(payload.arg1.v, payload.arg2.v);
                break;
            case multiply_op:
                v = e_m(payload.arg1.v, payload.arg2.v);
                break;
            }
            double dv = 0.0;
            prop_t r = ((prop_t){v, &dv});
            handle(k, &r);

            double x = payload.arg1.v;
            double y = payload.arg2.v;
            double *dx = payload.arg1.dv;
            double *dy = payload.arg2.dv;
            switch (payload.op) {
            case add_op:
                *dx = e_a(*dx, dv);
                *dy = e_a(*dy, dv);
                break;
            case multiply_op:
                *dx = e_a(*dx, e_m(y, dv));
                *dy = e_a(*dy, e_m(x, dv));
                break;
            }
            break;
        })
    }
}

void *reverse(seff_coroutine_t *toplevel_handler, void *args) {
    seff_coroutine_t *child = seff_coroutine_new(example, args);

    handle(child, NULL);

    printf("%lf\n", *x.dv);

    seff_coroutine_delete(child);

    return NULL;
}

void *evaluate(seff_coroutine_t *k, void *args) {
    double value;

    seff_eff_t *request = seff_handle(k, NULL, e_smooth);
    while (k->state != FINISHED) {
        switch (request->id) {
            CASE_EFFECT(request, e_ap0, {
                value = payload.value;
                request = seff_handle(k, (void *)&value, e_smooth);
                break;
            })
            CASE_EFFECT(request, e_ap1, {
                switch (payload.op) {
                case negate_op:
                    value = -payload.arg1;
                    break;
                }
                request = seff_handle(k, (void *)&value, e_smooth);
                break;
            })
            CASE_EFFECT(request, e_ap2, {
                double arg1 = payload.arg1;
                double arg2 = payload.arg2;
                switch (payload.op) {
                case add_op:
                    value = arg1 + arg2;
                    break;
                case multiply_op:
                    value = arg1 * arg2;
                    break;
                }
                request = seff_handle(k, (void *)&value, e_smooth);
                break;
            })
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    size_t iters = 100;
    if (argc == 2) {
        sscanf(argv[1], "%lu", &iters);
    }

    seff_coroutine_t *k = seff_coroutine_new(reverse, (void *)iters);

    evaluate(k, NULL);

    seff_coroutine_delete(k);

    return 0;
}
