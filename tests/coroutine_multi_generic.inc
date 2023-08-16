#include "scheduler.c"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

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
void *compute_square_root(seff_coroutine_t *self, void *_state) {
    coroutine_state state = *(coroutine_state *)_state;
    float guess = state.x;

    for (int i = 0; i < ITERS; i++) {
        if (i % (ITERS / YIELDS) == 0)
            PERFORM(yield);
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

    scheduler_t scheduler;
    scheduler_init(&scheduler, THREADS, TASK_QUEUE_SIZE);
    threadsafe_printf("Scheduler construction finished\n");
    threadsafe_printf("Creating workers\n");

    coroutine_state coroutine_states[SQUARE_ROOTS];
    for (int i = 0; i < SQUARE_ROOTS; i++) {
        coroutine_states[i].x = i;
        coroutine_states[i].location = &square_roots[i];
        scheduler_schedule(&scheduler, compute_square_root, &coroutine_states[i], i % THREADS);
    }
    threadsafe_printf("Worker creation finished\n");
    scheduler_start(&scheduler);
    scheduler_join(&scheduler);

    threadsafe_printf("Finished %d tasks out of %d\n", atomic_load(&finished), SQUARE_ROOTS);

    scheduler_destroy(&scheduler);
}
