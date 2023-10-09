
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#include "scheff.h"
#include "seff.h"

#define SQUARE_ROOTS 100000
#define ITERS 10000
#define YIELDS 10

float square_roots[SQUARE_ROOTS];
_Atomic int finished;

typedef struct {
    float x;
    float *location;
} coroutine_state;

bool const_true(void *_arg) { return true; }

void *compute_square_root(seff_coroutine_t *self, void *_state) {
    coroutine_state state = *(coroutine_state *)_state;
    float guess = state.x;

    for (int i = 0; i < ITERS; i++) {
        if (i % (ITERS / YIELDS) == 0) {
            scheff_poll(const_true, NULL);
        }
        guess = (guess + (state.x / guess)) / 2;
    }

    *state.location = guess;
    atomic_fetch_add(&finished, 1);
    return NULL;
}

#define THREADS 8
#define TASK_QUEUE_SIZE SQUARE_ROOTS

int main(void) {
    atomic_store(&finished, 0);

    scheff_t scheduler;
    scheff_init(&scheduler, THREADS);
    printf("Scheduler construction finished\n");
    printf("Creating workers\n");

    coroutine_state coroutine_states[SQUARE_ROOTS];
    for (int i = 0; i < SQUARE_ROOTS; i++) {
        coroutine_states[i].x = i;
        coroutine_states[i].location = &square_roots[i];
        if (!scheff_schedule(&scheduler, compute_square_root, &coroutine_states[i])) {
            printf("Something went wrong when scheduling the main task!\n");
            exit(1);
        }
    }
    printf("Worker creation finished\n");

    scheff_run(&scheduler);
    printf("Finished %d tasks out of %d\n", atomic_load(&finished), SQUARE_ROOTS);

#ifndef NDEBUG
    scheff_print_stats(&scheduler);
#endif
}
