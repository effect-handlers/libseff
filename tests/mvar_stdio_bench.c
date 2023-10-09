#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mvar_stdio.h"
#include "scheff.h"
#include "seff.h"

#define REPS 50000
#define THREADS 2
#define TASKS 32

void *printer(seff_coroutine_t *self, void *arg) {
    for (int i = 0; i < REPS; ++i) {
        // my_bench
        mvar_printf("Hello from %lu\n", (uintptr_t)arg);
        // posix_bench
        // printf("Hello from %lu\n", (uintptr_t)arg);
    }
    return NULL;
}
bool const_true(void *_arg) { return true; }

void *calculator(seff_coroutine_t *self, void *_arg) {
    size_t a = 0;
    size_t *arg = (size_t *)_arg;
    for (int i = 0; i < REPS; ++i) {
        a += arg[i];
        scheff_poll(const_true, NULL);
    }
    mvar_printf("Bye from %lu\n", (uintptr_t)a);
    // printf("Bye from %lu\n", (uintptr_t)a);

    return (void *)a;
}

int main(void) {
    scheff_t scheduler;
    scheff_init(&scheduler, THREADS);

    size_t a[REPS];
    for (int i = 0; i < REPS; ++i) {
        a[i] = i;
    }
    for (uintptr_t i = 0; i < TASKS; ++i) {
        if (!scheff_schedule(&scheduler, printer, (void *)i)) {
            exit(1);
        }
    }

    for (uintptr_t i = 0; i < 100 * TASKS; ++i) {
        if (!scheff_schedule(&scheduler, calculator, (void *)a)) {
            exit(1);
        }
    }

    scheff_run(&scheduler);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif
}
