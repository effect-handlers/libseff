#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mvar.h"
#include "seff.h"

typedef struct {
    mvar_t *mv;
    size_t els;
    size_t start;
} args;

void *producer(seff_coroutine_t *self, void *_arg) {
    args *arg = (args *)_arg;

    for (int i = 0; i < arg->els; i++) {
        printf("Putting [%lu]\n", i + arg->start);
        mvar_put(arg->mv, (void *)((uintptr_t)i + arg->start));
        printf("Put [%lu]\n", i + arg->start);
    }
    printf("Bye producer\n");

    return NULL;
}

void *consumer(seff_coroutine_t *self, void *_arg) {
    args *arg = (args *)_arg;

    for (int i = 0; i < arg->els; i++) {
        printf("Taking\n");
        size_t x = (size_t)mvar_take(arg->mv);
        printf("Took %zu\n", x);
    }
    printf("Bye consumer\n");

    return NULL;
}

#define PRODUCERS 10
#define CONSUMERS 10
#define ELS 5

int main(void) {
    scheff_t scheduler;
    scheff_init(&scheduler, 3);
    mvar_t mv;
    mvar_init(&mv);

    args prod_args[PRODUCERS];
    for (int i = 0; i < PRODUCERS; ++i) {
        prod_args[i].mv = &mv;
        prod_args[i].els = ELS;
        prod_args[i].start = 1 + i * ELS;

        if (!scheff_schedule(&scheduler, producer, &prod_args[i])) {
            printf("Something went wrong when scheduling the main task!\n");
            exit(1);
        }
    }

    args cons_args[CONSUMERS];
    for (int i = 0; i < CONSUMERS; ++i) {
        cons_args[i].mv = &mv;
        cons_args[i].els = ELS;
        cons_args[i].start = 1 + i * ELS;

        if (!scheff_schedule(&scheduler, consumer, &cons_args[i])) {
            printf("Something went wrong when scheduling the main task!\n");
            exit(1);
        }
    }

    scheff_run(&scheduler);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif
}
